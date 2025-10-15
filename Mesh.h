//------------------------------------------------------- ----------------------
// File: Mesh.h
//-----------------------------------------------------------------------------

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CVertex
{
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
    CVertex(float x, float y, float z) { m_xmf3Position = XMFLOAT3(x, y, z); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() { }

	XMFLOAT3						m_xmf3Position;
};

class CPolygon
{
public:
    CPolygon() {}
    CPolygon(int nVertices);
    ~CPolygon();

    int							m_nVertices = 0;
    CVertex* m_pVertices = NULL;

    void SetVertex(int nIndex, CVertex& vertex);
};

class CDiffusedVertex : public CVertex {
public:
	XMFLOAT4 m_xmf4Diffuse;
	XMFLOAT3 m_xmf3Normal;
	CDiffusedVertex() : m_xmf4Diffuse(0, 0, 0, 0), m_xmf3Normal(0, 1, 0) {}
	CDiffusedVertex(XMFLOAT3 pos, XMFLOAT4 dif, XMFLOAT3 normal) : CVertex(pos), m_xmf4Diffuse(dif), m_xmf3Normal(normal) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMesh
{
public:
	CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName = NULL);
	CMesh(int nPolygons);
	virtual ~CMesh();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void ReleaseUploadBuffers();
    void SetPolygon(int nIndex, CPolygon* pPolygon);
	int CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance = nullptr);
	BOOL RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance);


	BoundingBox						m_xmBoundingBox;
	BoundingOrientedBox			    m_xmOOBB = BoundingOrientedBox();
protected:
	UINT							m_nVertices = 0;
	XMFLOAT3						*m_pxmf3Positions = NULL;
	ID3D12Resource					*m_pd3dPositionBuffer = NULL;
	ID3D12Resource					*m_pd3dPositionUploadBuffer = NULL;

	XMFLOAT3						*m_pxmf3Normals = NULL;
	ID3D12Resource					*m_pd3dNormalBuffer = NULL;
	ID3D12Resource					*m_pd3dNormalUploadBuffer = NULL;

	XMFLOAT2						*m_pxmf2TextureCoords = NULL;
	ID3D12Resource					*m_pd3dTextureCoordBuffer = NULL;
	ID3D12Resource					*m_pd3dTextureCoordUploadBuffer = NULL;

	UINT							m_nIndices = 0;
	UINT							*m_pnIndices = NULL;
	ID3D12Resource					*m_pd3dIndexBuffer = NULL;
	ID3D12Resource					*m_pd3dIndexUploadBuffer = NULL;

	UINT							m_nVertexBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW		*m_pd3dVertexBufferViews = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dVertexBufferView;

	D3D12_INDEX_BUFFER_VIEW			m_d3dIndexBufferView;

	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nStride = 0;
	UINT							m_nOffset = 0;

	UINT							m_nStartIndex = 0;
	int								m_nBaseVertex = 0;

    int							    m_nPolygons = 0;
    CPolygon                        **m_ppPolygons = NULL;

public:
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);

	void LoadMeshFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, char *pstrFileName);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCubeMesh : public CMesh
{
public:
	CCubeMesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);

	virtual ~CCubeMesh();
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CHeightMapImage
{
private:
	BYTE *m_pHeightMapPixels;
	int m_nWidth;
	int m_nLength;
	XMFLOAT3 m_xmf3Scale;
public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage(void);
	float GetHeight(float x, float z);
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	BYTE* GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

class CHeightMapGridMesh : public CMesh
{
protected:
	int m_nWidth;
	int m_nLength;
	XMFLOAT3 m_xmf3Scale;
public:
	CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale =
		XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void
		* pContext = NULL);
	virtual ~CHeightMapGridMesh();
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }
	virtual float OnGetHeight(int x, int z, void *pContext);
	virtual XMFLOAT4 OnGetColor(int x, int z, void *pContext);
};