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
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[4];
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

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[3].Constants.Num32BitValues = 6; // float 6개 + 패딩 2개
	pd3dRootParameters[3].Constants.ShaderRegister = 3; // b3 
	pd3dRootParameters[3].Constants.RegisterSpace = 0;
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = 4;
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

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
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
}
void CScene::BuildGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
}

//탱크 Scene////////////////////////////////////////////////////////////////////////////////////////////////
CTankScene::CTankScene(CPlayer* pPlayer) : CScene(pPlayer) {}
void CTankScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	CLightingShader* pShader = new CLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


	CTerrainLightingShader* pTerrainShader = new CTerrainLightingShader();
	pTerrainShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	using namespace std;
	default_random_engine dre{ random_device{}() };
	uniform_real_distribution<float> uid{ 0.0f,1.0f };

	uniform_real_distribution<float> uid_x{ 0,100.0f };
	uniform_real_distribution<float> uid_z{ 0,100.0f };
	uniform_real_distribution<float> uid_rot{ 0,360.0f };

	for (int i = 0; i < m_nTanks; i++)
	{
		float red = uid(dre);
		float green = uid(dre);
		float blue = uid(dre);

		m_pTank[i] = nullptr;
		m_pTank[i] = new CTankObject();
		CMesh* pTankMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Tank.obj");
		m_pTank[i]->SetMesh(0,pTankMesh);
		m_pTank[i]->SetShader(pShader);
		m_pTank[i]->SetColor(XMFLOAT3(red, green, blue));
		m_pTank[i]->SetPosition(uid_x(dre) - 50.0f, 0.0f, uid_z(dre) - 50.0f);
		m_pTank[i]->Rotate(0.0f, uid_rot(dre), 0.0f);
		m_pTank[i]->UpdateBoundingBox();

		m_pTank[i]->bullet = new CGameObject();
		CMesh* pMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Bullet.obj");
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
	}

	XMFLOAT3 xmf3Scale(1.0f, 0.2f, 1.0f);
	XMFLOAT4 xmf4Color(0.2f, 0.2f, 0.2f, 0.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList,
		m_pd3dGraphicsRootSignature, _T("Models/HeightMap.raw"), 257, 257, 257,
		257, xmf3Scale, xmf4Color);
	m_pTerrain->SetPosition(-128.0f, -20.0f, -128.0f);
	m_pTerrain->SetColor(XMFLOAT3(0.2f, 0.2f, 0.2f));
	m_pTerrain->SetShader(pTerrainShader);
	m_pTerrain->UpdateBoundingBox();
}

void CTankScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i]->bullet)delete m_pTank[i]->bullet;
		if (m_pTank[i]->m_pExplosionObjects)delete m_pTank[i]->m_pExplosionObjects;
		if (m_pTank[i])delete m_pTank[i];
	}
	if (m_pTerrain) delete m_pTerrain;
}
void CTankScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i]) m_pTank[i]->ReleaseUploadBuffers();
		if (m_pTank[i]->m_pExplosionObjects) m_pTank[i]->m_pExplosionObjects->ReleaseUploadBuffers();
	}
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
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
	pd3dCommandList->SetGraphicsRoot32BitConstants(3, 6, light, 0);

	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);
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
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
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
		case 'Q':
			if (pTankPlayer) {
				pTankPlayer->SwitchShild();
			}
			break;
		case 'E':
			if (pTankPlayer && !pTankPlayer->shot) {
				pTankPlayer->SwitchBullet();
				pTankPlayer->SetBulletPosition();
			}
			break;
			break;
		case VK_RETURN:
			if (pTankPlayer->Toggle) {
				pTankPlayer->Toggle = false;
				m_pTerrain->SetColor(XMFLOAT3(0.2f, 0.2f, 0.2f));
			}
			else {
				pTankPlayer->Toggle = true;
				m_pTerrain->SetColor(XMFLOAT3(0.3f, 0.0f, 0.0f));
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
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
	{
		if (pTankPlayer->Toggle) {

			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			CCamera* pCamera = m_pPlayer->GetCamera();

			CGameObject* pPickedObject = PickObjectPointedByCursor(x, y, pCamera);

			if (pPickedObject) {
				for (int i = 0; i < m_nTanks; i++) {
					if (pPickedObject == m_pTank[i]) {
						pTankPlayer->ToggleObject = m_pTank[i];
					}
				}
			}

			break;
		}
	}
	}
}
void CTankScene::CheckTankByBulletCollisions()
{
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	if (pTankPlayer && pTankPlayer->shot) {
		for (int i = 0; i < 10; i++)
		{
			if (m_pTank[i]->IsExist())
				if (m_pTank[i]->m_xmOOBB.Intersects(pTankPlayer->m_pBullet->m_xmOOBB))
				{
					if (!m_pTank[i]->IsBlowingUp()) {
						m_pTank[i]->PrepareExplosion();
						pTankPlayer->shot = false;
						pTankPlayer->bullet_timer = 0;
						pTankPlayer->ToggleObject = NULL;
					}
				}
		}
	}
}
void CTankScene::CheckPlayerByBulletCollisions()
{
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	if (pTankPlayer) {
		for (int i = 0; i < m_nTanks; i++)
		{
			if (!pTankPlayer->OnShild) {
				if (m_pTank[i]->IsExist() && m_pTank[i]->IsShot())
					if (pTankPlayer->m_xmOOBB.Intersects(m_pTank[i]->bullet->m_xmOOBB))
					{
						XMFLOAT3 color = m_pTank[i]->m_xmf3Color;

						pTankPlayer->SetColor(color);
						pTankPlayer->m_pBullet->SetColor(color);
						pTankPlayer->m_pShild->SetColor(color);

						m_pTank[i]->SwitchShot();
					}
			}
			else {
				if (m_pTank[i]->IsExist() && m_pTank[i]->IsShot())
					if (pTankPlayer->m_pShild->m_xmOOBB.Intersects(m_pTank[i]->bullet->m_xmOOBB))
					{
						m_pTank[i]->SwitchShot();
					}
			}
		}
	}
}

void CTankScene::CheckExpByTerrain()
{
	for (int i = 0; i < 10; i++)
	{
		if (m_pTank[i]->IsExist() && m_pTank[i]->IsBlowingUp()) {
			for (int j = 0; j < EXPLOSION_DEBRISES; j++)
			if (m_pTank[i]->m_pExplosionObjects->Draw[j])
			{
				XMFLOAT3 xmf3Position;
				XMFLOAT4X4& xmf4x4Transform = m_pTank[i]->m_pxmf4x4Transforms[j];
				xmf3Position = XMFLOAT3(xmf4x4Transform._41, xmf4x4Transform._42, xmf4x4Transform._43);

				float fHeight = m_pTerrain->GetHeight(xmf3Position);
				if (xmf3Position.y < fHeight) {
					m_pTank[i]->m_pExplosionObjects->Draw[j] = false;
					m_pTank[i]->m_pExplosionObjects->m_pxmf4x4Transforms[j] = Matrix4x4::Identity();
					m_pTank[i]->m_pExplosionObjects->m_pxmf3SphereVectors[j] = XMFLOAT3(0.0f, 0.0f, 0.0f);
				}
			}
		}
	}
}

void CTankScene::CheckBulletByTerrain()
{
	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);
	if (pTankPlayer && pTankPlayer->shot) {
		XMFLOAT3 xmf3Position;
		xmf3Position = pTankPlayer->m_pBullet->GetPosition();

		float fHeight = m_pTerrain->GetHeight(xmf3Position);
		if (xmf3Position.y + 1.0f < fHeight) {
			pTankPlayer->shot = false;
			pTankPlayer->bullet_timer = 0;
			pTankPlayer->ToggleObject = NULL;
		}
	}
}

void CTankScene::Animate(float fElapsedTime)
{
	for (int i = 0; i < m_nTanks; i++) {
		if (m_pTank[i]) {

			m_pTank[i]->Animate(fElapsedTime);
			XMFLOAT3 xmf3Position = m_pTank[i]->GetPosition();
			m_pTank[i]->Height = m_pTerrain->GetHeight(xmf3Position);
			m_pTank[i]->Fall(G, m_pTerrain->GetNormal(xmf3Position));

			if (m_pTank[i]->IsBlowingUp()) {
				for (int j = 0; j < EXPLOSION_DEBRISES; j++) {
					m_pTank[i]->m_pExplosionObjects->m_pxmf4x4Transforms[j] = m_pTank[i]->m_pxmf4x4Transforms[j];
					m_pTank[i]->m_pExplosionObjects->m_pxmf3SphereVectors[j] = m_pTank[i]->m_pxmf3SphereVectors[j];
				}
			}
		}
	}

	CTankPlayer* pTankPlayer = dynamic_cast<CTankPlayer*>(m_pPlayer);

	XMFLOAT3 xmf3Position = pTankPlayer->GetPosition();
	pTankPlayer->Height = m_pTerrain->GetHeight(xmf3Position);
	pTankPlayer->Fall(G, m_pTerrain->GetNormal(xmf3Position));
	pTankPlayer->Animate(fElapsedTime);

	CheckTankByBulletCollisions();
	CheckPlayerByBulletCollisions();
	CheckExpByTerrain();
	CheckBulletByTerrain();
}