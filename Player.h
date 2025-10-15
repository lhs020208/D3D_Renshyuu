#pragma once

#include "Object.h"
#include "Camera.h"

class CPlayer : public CGameObject
{
public:
	XMFLOAT3					m_xmf3Position;
	XMFLOAT3					m_xmf3Right;
	XMFLOAT3					m_xmf3Up;
	XMFLOAT3					m_xmf3Look;
	XMFLOAT3					m_xmf3Color = XMFLOAT3(0.5f, 0.5f, 0.5f);

	float           			m_fPitch;
	float           			m_fYaw;
	float           			m_fRoll;

	XMFLOAT3					m_xmf3CameraOffset = XMFLOAT3(0.0f, 0.0f, -4.0f);
	XMFLOAT3					m_xmf3Velocity;
	XMFLOAT3     				m_xmf3Gravity;
	float           			m_fMaxVelocityXZ;
	float           			m_fMaxVelocityY;
	float           			m_fFriction;

	LPVOID						m_pPlayerUpdatedContext;
	LPVOID						m_pCameraUpdatedContext;

	CCamera						*m_pCamera = NULL;
	bool						overview = false;

	int							move_z = 0;
	int							move_x = 0;
public:
	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetPosition(float x, float y, float z);
	void SetCameraOffset(XMFLOAT3& xmf3CameraOffset);

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	void Update(float fTimeElapsed);
	virtual void Animate(float fElapsedTime) { }
	void reset();

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void Fall(float G, XMFLOAT3 Normal) {}
	XMFLOAT3 LastUpVector = GetUp();  // �ʱ�ȭ �ʿ�
};

class CCubePlayer : public CPlayer
{
public:
	CCubePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CCubePlayer();

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
};

class CTankPlayer : public CPlayer
{
public:
	CTankPlayer() {}
	CTankPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~CTankPlayer() {}

	virtual void OnPrepareRender();
	virtual void Animate(float fElapsedTime) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f) override;
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	bool OnShild = false;
	void SwitchShild() { OnShild = !OnShild; }
	bool shot = false;
	void SwitchBullet() { shot = !shot; }
	int bullet_timer = 0;
	void CTankPlayer::SetBulletPosition();

	CCubeObject* m_pShild;
	CGameObject* m_pBullet;

	CTankObject* ToggleObject = nullptr;
	bool Toggle = false;
private:
	XMFLOAT3 m_xmf3MoveVector = { 0.0f, 0.0f, 0.0f };

public:
	const XMFLOAT3& GetMoveVector() const { return m_xmf3MoveVector; }
	void ClearMoveVector() { m_xmf3MoveVector = { 0.0f, 0.0f, 0.0f }; }

	float FallingSpeed = 0.0f;
	float Height;
	virtual void Fall(float G, XMFLOAT3 Normal)override;

};
