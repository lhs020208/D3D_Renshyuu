#pragma once
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

struct BoneTransform {
    int        parent = -1;           // �θ� �ε���(-1�̸� ��Ʈ)
    XMFLOAT4X4 inverseBind{};         // inverse(bindPose)
    XMFLOAT4X4 local{};               // ���� ������ ����(S*R*T)
    XMFLOAT4X4 global{};              // ���� ���� ����
};

struct Keyframe {
    float     t = 0.f;
    XMFLOAT3  T = { 0,0,0 };
    XMFLOAT4  R = { 0,0,0,1 };          // quaternion(x,y,z,w)
    XMFLOAT3  S = { 1,1,1 };
};
struct BoneTrack {
    std::vector<Keyframe> keys;       // �ð� ��������
};

class CAnimationClip {
public:
    float duration = 0.f;
    std::vector<BoneTrack> tracks;    // [boneIndex]

    // time�� ���� ���� local(S*R*T) ����� out[i].local�� ���
    void Evaluate(float time, std::vector<BoneTransform>& out) const;
};

class CAnimator {
public:
    std::vector<BoneTransform> bones; // skeleton + ��Ÿ�� ��
    CAnimationClip* clip = nullptr;
    float time = 0.f;

    void  SetSkeleton(const std::vector<BoneTransform>& s) { bones = s; }
    void  SetClip(CAnimationClip* c) { clip = c; time = 0.f; }
    void  Update(float dt);                      // time ���� + Evaluate + ���� ����
    std::vector<XMFLOAT4X4> GetSkinMatrices() const; // skin = global * inverseBind (���̴���)
};
