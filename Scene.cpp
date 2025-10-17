//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"
#include "GameFramework.h"
extern CGameFramework* g_pFramework;

CScene::CScene(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
}

CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	// **중요**: 파라미터 배열 전체를 0으로 초기화해서 미정의된 필드(garbage) 방지
	D3D12_ROOT_PARAMETER pd3dRootParameters[5];
	::ZeroMemory(pd3dRootParameters, sizeof(pd3dRootParameters));
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 4; //Time, ElapsedTime, xCursor, yCursor
	pd3dRootParameters[0].Constants.ShaderRegister = 0; //Time
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 19; //16 + 3
	pd3dRootParameters[1].Constants.ShaderRegister = 1; //World
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 35; //16 + 16 + 3
	pd3dRootParameters[2].Constants.ShaderRegister = 2; //Camera
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[3].Descriptor.ShaderRegister = 3;  // register(b3)
	pd3dRootParameters[3].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // b4
	pd3dRootParameters[4].Descriptor.ShaderRegister = 4;  // register(b4)
	pd3dRootParameters[4].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = 5;
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	if (FAILED(hr))
	{
		if (pd3dErrorBlob)
		{
			OutputDebugStringA((const char*)pd3dErrorBlob->GetBufferPointer()); // 에러 메시지 출력
			pd3dErrorBlob->Release();
		}
		if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
		return nullptr; // 실패하면 null 반환
	}

	hr = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	if (FAILED(hr))
	{
		// CreateRootSignature 실패 로그
		OutputDebugStringA("CreateRootSignature failed\n");
		return nullptr;
	}

	return(pd3dGraphicsRootSignature);
}

void CScene::ReleaseObjects()
{
	
}

void CScene::ReleaseUploadBuffers()
{

}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}
void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

bool CScene::ProcessInput()
{
	return(false);
}

void CScene::Animate(float fTimeElapsed)
{
}

void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dGraphicsRootSignature)
	{
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	}
	else
	{
		OutputDebugStringA("Warning: m_pd3dGraphicsRootSignature is null in PrepareRender\n");
		// (필요시 assert 또는 빠른 복구 로직 추가)
	}
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
}
void CScene::BuildGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
}

void CScene::CreateLightConstantBuffer(ID3D12Device* device)
{
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = 256; // 256바이트 정렬
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_pLightCB)
	);
}

//탱크 Scene////////////////////////////////////////////////////////////////////////////////////////////////
CTankScene::CTankScene(CPlayer* pPlayer) : CScene(pPlayer) {}

struct LIGHT_CB
{
	XMFLOAT3 LightDirection; float pad1 = 0.0f;
	XMFLOAT3 LightColor;     float pad2 = 0.0f;
};

void CTankScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	CLightingShader* pShader = new CLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	using namespace std;

	LIGHT_CB lightData = { m_xmf3LightDirection, 0.0f, m_xmf3LightColor, 0.0f };

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	// 리소스 디스크립터
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = 0;
	resDesc.Width = 256;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// 리소스 생성
	HRESULT hr = pd3dDevice->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_pLightCB)
	);
	if (FAILED(hr)) OutputDebugString(L"CreateCommittedResource for LightCB failed!\n");

	// 데이터 복사 (한 번만)
	void* pMapped = nullptr;
	m_pLightCB->Map(0, nullptr, &pMapped);
	memcpy(pMapped, &lightData, sizeof(LIGHT_CB));
	m_pLightCB->Unmap(0, nullptr);

	default_random_engine dre{ random_device{}() };
	uniform_real_distribution<float> uid{ 0.0f,1.0f };

	uniform_real_distribution<float> uid_x{ 0,100.0f };
	uniform_real_distribution<float> uid_z{ 0,100.0f };
	uniform_real_distribution<float> uid_rot{ 0,360.0f };

	CMesh* pPlayerMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/unitychan.bin", 2);
	m_pPlayer->SetMesh(0, pPlayerMesh);
	m_pPlayer->SetPosition(0.0f, 6.0f, 0.0f);
	m_pPlayer->SetCameraOffset(XMFLOAT3(0.0f, 6.0f, -12.0f));

	for (int i = 0; i < m_nTanks; i++)
	{
		float red = uid(dre);
		float green = uid(dre);
		float blue = uid(dre);

		m_pTank[i] = nullptr;
		m_pTank[i] = new CTankObject();
		CMesh* pTankMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Tank.obj", 1);
		m_pTank[i]->SetMesh(0,pTankMesh);
		m_pTank[i]->SetShader(pShader);
		m_pTank[i]->SetColor(XMFLOAT3(red, green, blue));
		m_pTank[i]->SetPosition(uid_x(dre) - 50.0f, 0.0f, uid_z(dre) - 50.0f);
		m_pTank[i]->Rotate(0.0f, uid_rot(dre), 0.0f);
		m_pTank[i]->UpdateBoundingBox();

		m_pTank[i]->bullet = new CGameObject();
		CMesh* pMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Bullet.obj", 1);
		m_pTank[i]->bullet->SetMesh(0,pMesh);
		m_pTank[i]->bullet->SetColor(XMFLOAT3(red, green, blue));
		m_pTank[i]->bullet->SetPosition(-2.0f + 0.5f * i, 0.0f, 1.0f);
		m_pTank[i]->bullet->SetShader(pShader);
		m_pTank[i]->bullet->UpdateBoundingBox();

		CCubeMesh* pCubeMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 0.25f, 0.25f, 0.25f);
		m_pTank[i]->m_pExplosionObjects = new CExplosionObject();
		m_pTank[i]->m_pExplosionObjects->SetMesh(0,pCubeMesh);
		m_pTank[i]->m_pExplosionObjects->SetShader(pShader);
		m_pTank[i]->m_pExplosionObjects->SetColor(XMFLOAT3(red, green, blue));
		m_pTank[i]->m_pExplosionObjects->SetPosition(0.0f, 0.0f, 1.0f);
		m_pTank[i]->m_pExplosionObjects->UpdateBoundingBox();

		for (int j = 0; j < EXPLOSION_DEBRISES; j++) {
			m_pTank[i]->m_pExplosionObjects->Draw[j] = false;
		}

		if (!m_pDefaultBoneCB) {
			// 128개의 아이덴티티 행렬(스키닝 미사용 시에도 안전)
			const UINT maxBones = 128;
			std::vector<XMFLOAT4X4> id(maxBones);
			for (auto& m : id) m = Matrix4x4::Identity(); // 본인 유틸 있으면 사용

			auto Align256 = [](UINT sz) { return (sz + 255u) & ~255u; };
			const UINT cbSize = Align256(sizeof(XMFLOAT4X4) * maxBones);

			m_pDefaultBoneCB = CreateBufferResource(pd3dDevice, pd3dCommandList,
				id.data(), cbSize,
				D3D12_HEAP_TYPE_UPLOAD,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr);
		}
	}
}

void CTankScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i]->bullet)delete m_pTank[i]->bullet;
		if (m_pTank[i]->m_pExplosionObjects)delete m_pTank[i]->m_pExplosionObjects;
		if (m_pTank[i])delete m_pTank[i];
	}
}
void CTankScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i]) m_pTank[i]->ReleaseUploadBuffers();
		if (m_pTank[i]->m_pExplosionObjects) m_pTank[i]->m_pExplosionObjects->ReleaseUploadBuffers();
	}
}


void CTankScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);
	float light[6] = {
		m_xmf3LightDirection.x, 
		m_xmf3LightDirection.y, 
		m_xmf3LightDirection.z, 
		m_xmf3LightColor.x, 
		m_xmf3LightColor.y,
		m_xmf3LightColor.z, 
	};

	// 렌더 시 바인딩
	if (m_pLightCB)
		pd3dCommandList->SetGraphicsRootConstantBufferView(3, m_pLightCB->GetGPUVirtualAddress());
	if (m_pPlayer) m_pPlayer->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nTanks; i++) {
		
		if (m_pTank[i]->IsExist()) {
			if (m_pTank[i]->IsBlowingUp()) {
				m_pTank[i]->m_pExplosionObjects->Render(pd3dCommandList, pCamera);
			}
			else {
				m_pTank[i]->Render(pd3dCommandList, pCamera);
			}
		}
	}
}
void CTankScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	extern CGameFramework* g_pFramework;
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W':
			if (m_pPlayer->move_z < 1)m_pPlayer->move_z += 1;
			break;
		case 'S':
			if (m_pPlayer->move_z > -1)m_pPlayer->move_z -= 1;
			break;
		case 'A':
			if (m_pPlayer->move_x > -1)m_pPlayer->move_x -= 1;
			break;
		case 'D':
			if (m_pPlayer->move_x < 1)m_pPlayer->move_x += 1;
			break;
		case VK_SPACE:
			for (int i = 0; i < 10; i++)
				if (!m_pTank[i]->IsBlowingUp()) {
					m_pTank[i]->PrepareExplosion();
				}
			break;
		default:
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case 'W':
			if (m_pPlayer->move_z > -1)m_pPlayer->move_z -= 1;
			break;
		case 'S':
			if (m_pPlayer->move_z < 1)m_pPlayer->move_z += 1;
			break;
		case 'A':
			if (m_pPlayer->move_x < 1)m_pPlayer->move_x += 1;
			break;
		case 'D':
			if (m_pPlayer->move_x > -1)m_pPlayer->move_x -= 1;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}
CGameObject* CTankScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{

	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / (float)pCamera->m_d3dViewport.Width) - 1) / pCamera->m_xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / (float)pCamera->m_d3dViewport.Height) - 1) / pCamera->m_xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	float fNearestHitDistance = FLT_MAX;
	CGameObject* pNearestObject = NULL;
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i])
		{
			int hit = m_pTank[i]->PickObjectByRayIntersection(xmvPickPosition, xmmtxView, &fNearestHitDistance);
			if (hit > 0)
			{
				pNearestObject = m_pTank[i];
			}
		}
	}
	return(pNearestObject);

}
void CTankScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void CTankScene::Animate(float fElapsedTime)
{
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i]) {

			m_pTank[i]->Animate(fElapsedTime);
			XMFLOAT3 xmf3Position = m_pTank[i]->GetPosition();

			if (m_pTank[i]->IsBlowingUp()) {
				for (int j = 0; j < EXPLOSION_DEBRISES; j++) {
					m_pTank[i]->m_pExplosionObjects->m_pxmf4x4Transforms[j] = m_pTank[i]->m_pxmf4x4Transforms[j];
					m_pTank[i]->m_pExplosionObjects->m_pxmf3SphereVectors[j] = m_pTank[i]->m_pxmf3SphereVectors[j];
				}
			}
		}
	}

	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	m_pPlayer->Animate(fElapsedTime);
}