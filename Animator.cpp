#include "stdafx.h"
#include "Animator.h"
#include <algorithm>

static inline XMMATRIX ComposeTRS(const XMFLOAT3& T, const XMFLOAT4& R, const XMFLOAT3& S)
{
    return XMMatrixScaling(S.x, S.y, S.z) *
        XMMatrixRotationQuaternion(XMLoadFloat4(&R)) *
        XMMatrixTranslation(T.x, T.y, T.z);
}

static inline void LerpKey(const Keyframe& k0, const Keyframe& k1, float t01,
    XMFLOAT3& T, XMFLOAT4& R, XMFLOAT3& S)
{
    XMStoreFloat3(&T, XMVectorLerp(XMLoadFloat3(&k0.T), XMLoadFloat3(&k1.T), t01));
    XMStoreFloat4(&R, XMQuaternionSlerp(XMLoadFloat4(&k0.R), XMLoadFloat4(&k1.R), t01));
    XMStoreFloat3(&S, XMVectorLerp(XMLoadFloat3(&k0.S), XMLoadFloat3(&k1.S), t01));
}

void CAnimationClip::Evaluate(float time, std::vector<BoneTransform>& out) const
{
    if (tracks.empty()) return;
    if (out.size() < tracks.size()) out.resize(tracks.size());

    // 루프 애니
    float t = duration > 0.f ? fmodf(time, duration) : time;

    for (size_t i = 0; i < tracks.size(); ++i)
    {
        const auto& tr = tracks[i];
        if (tr.keys.empty()) {
            XMStoreFloat4x4(&out[i].local, XMMatrixIdentity());
            continue;
        }
        if (tr.keys.size() == 1) {
            const auto& k = tr.keys[0];
            XMStoreFloat4x4(&out[i].local, ComposeTRS(k.T, k.R, k.S));
            continue;
        }
        // 구간 탐색
        size_t k = 0;
        while (k + 1 < tr.keys.size() && tr.keys[k + 1].t <= t) ++k;
        if (k + 1 >= tr.keys.size()) { // 마지막 구간 넘음 -> 마지막 키 유지
            const auto& last = tr.keys.back();
            XMStoreFloat4x4(&out[i].local, ComposeTRS(last.T, last.R, last.S));
            continue;
        }

        const auto& k0 = tr.keys[k];
        const auto& k1 = tr.keys[k + 1];
        float denom = (std::max)(1e-6f, (k1.t - k0.t));
        float a = (t - k0.t) / denom;  // 0..1

        XMFLOAT3 T; XMFLOAT4 R; XMFLOAT3 S;
        LerpKey(k0, k1, a, T, R, S);
        XMStoreFloat4x4(&out[i].local, ComposeTRS(T, R, S));
    }
}

void CAnimator::Update(float dt)
{
    if (!clip || bones.empty()) return;

    time += dt;
    clip->Evaluate(time, bones); // local 채움


    // 계층 누적: global = (parent==-1 ? local : local * global[parent])
    for (int idx : order) {
        XMMATRIX L = XMLoadFloat4x4(&bones[idx].local);
        if (bones[idx].parent < 0) {
            XMStoreFloat4x4(&bones[idx].global, L);
        }
        else {
            XMMATRIX P = XMLoadFloat4x4(&bones[bones[idx].parent].global);
            XMStoreFloat4x4(&bones[idx].global, XMMatrixMultiply(P, L));
        }
    }
}

std::vector<XMFLOAT4X4> CAnimator::GetSkinMatrices() const
{
    std::vector<XMFLOAT4X4> out;
    out.resize(bones.size());
    for (size_t i = 0; i < bones.size(); ++i)
    {
        XMMATRIX G = XMLoadFloat4x4(&bones[i].global);
        XMMATRIX IB = XMLoadFloat4x4(&bones[i].inverseBind);
        XMMATRIX M = G * IB;                   // skin = global * inverseBind

        // HLSL에서 mul(pos, M)을 쓰는 경우 보통 열-주도 매칭을 위해 전치가 필요
        XMStoreFloat4x4(&out[i], XMMatrixTranspose(M));
    }
    return out;
}

CAnimationClip* LoadAnimBIN(const char* path, const std::vector<CMesh::FBXBone>& meshBones) {
    std::ifstream in(path, std::ios::binary);
    uint32_t magic, ver, boneCount; in.read((char*)&magic, 4); in.read((char*)&ver, 4); in.read((char*)&boneCount, 4);
    std::vector<std::string> animBoneNames(boneCount);
    std::vector<int> animParent(boneCount);
    for (uint32_t i = 0; i < boneCount; ++i) {
        uint32_t nlen; in.read((char*)&nlen, 4); animBoneNames[i].resize(nlen);
        in.read(animBoneNames[i].data(), nlen);
        in.read((char*)&animParent[i], 4);
    }
    uint32_t animCount; in.read((char*)&animCount, 4);

    std::unordered_map<std::string, int> meshIndexByName;
    for (int i = 0; i < (int)meshBones.size(); ++i) meshIndexByName[meshBones[i].name] = i;
    std::vector<int> mapAnimToMesh(boneCount, -1);
    for (uint32_t i = 0; i < boneCount; ++i) {
        auto it = meshIndexByName.find(animBoneNames[i]);
        if (it != meshIndexByName.end()) mapAnimToMesh[i] = it->second;
    }

    uint32_t nlen; std::string aName; float fps; uint32_t numFrames;
    in.read((char*)&nlen, 4); aName.resize(nlen); in.read(aName.data(), nlen);
    in.read((char*)&fps, 4); in.read((char*)&numFrames, 4);

    auto* clip = new CAnimationClip();
    clip->tracks.resize(meshBones.size());
    clip->duration = (fps > 0.f) ? (numFrames / fps) : 0.f;

    for (uint32_t f = 0; f < numFrames; ++f) {
        float t = (fps > 0.f) ? (f / fps) : 0.f;
        for (uint32_t ai = 0; ai < boneCount; ++ai) {
            float T[3], Rdeg[3], S[3];
            in.read((char*)T, sizeof(float) * 3);
            in.read((char*)Rdeg, sizeof(float) * 3);
            in.read((char*)S, sizeof(float) * 3);

            int mi = mapAnimToMesh[ai];
            if (mi < 0) continue;

            // ↓ 이 위치에서 Keyframe k 선언/대입
            Keyframe k{};
            k.t = t;
            const float UNIT = 0.01f;                // 단위 스케일 (메시와 일치)
            k.T = XMFLOAT3(T[0] * UNIT, T[1] * UNIT, T[2] * UNIT);

            // degrees -> quaternion
            const float D2R = XM_PI / 180.f;
            XMVECTOR q = XMQuaternionRotationRollPitchYaw(Rdeg[0] * D2R, Rdeg[1] * D2R, Rdeg[2] * D2R);
            XMStoreFloat4(&k.R, q);

            k.S = XMFLOAT3(S[0], S[1], S[2]);
            clip->tracks[mi].keys.push_back(k);
        }
    }
    return clip;
}

void CAnimator::BuildParentFirstOrder() {
    order.clear();
    const int n = (int)bones.size();
    std::function<void(int)> dfs = [&](int u) {
        order.push_back(u);                    // 부모 먼저
        for (int i = 0; i < n; ++i)
            if (bones[i].parent == u) dfs(i);  // 자식들
        };
    for (int i = 0; i < n; ++i) if (bones[i].parent < 0) dfs(i); // 모든 루트에서 시작
}