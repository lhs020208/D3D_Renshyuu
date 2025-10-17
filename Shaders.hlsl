
cbuffer cbFrameworkInfo : register(b0)
{
	float 		gfCurrentTime;
	float		gfElapsedTime;
	float2		gf2CursorPos;
};

cbuffer cbGameObjectInfo : register(b1)
{
	matrix		gmtxWorld : packoffset(c0);
	float3		gf3ObjectColor : packoffset(c4);
};

cbuffer cbCameraInfo : register(b2)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gf3CameraPosition : packoffset(c8);
};

cbuffer cbLightInfo : register(b3)
{
    float gfLightDirectionX;
    float gfLightDirectionY;
    float gfLightDirectionZ;
	
    float gf3LightColorX;
    float gf3LightColorY;
    float gf3LightColorZ;
};
cbuffer cbBones : register(b4)
{
    float4x4 gBoneTransforms[128];
};

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXTURECOORD;
    uint4 boneIndices : BLENDINDICES; // �߰�
    float4 boneWeights : BLENDWEIGHT; // �߰�
};

struct VS_OUTPUT
{
	float4		positionH : SV_POSITION;
	float3		positionW : POSITION;
	float3		normal : NORMAL0;
	float3		normalW : NORMAL1;
	float2		uv : TEXTURECOORD;
};
float4 VSPseudoLighting(float4 position : POSITION) : SV_POSITION
{
    return position;
}

VS_OUTPUT VSLighting(VS_INPUT input)
{
	VS_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.positionH = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxWorld).xyz;
	output.normal = input.normal;
	output.uv = input.uv;

	return(output);
}

static float3 gf3AmbientLightColor = float3(1.0f, 1.0f, 1.0f);
static float3 gf3AmbientSpecularColor = float3(1.0f, 1.0f, 1.0f);

static float3 gf3SpecularColor = float3(0.2f, 0.2f, 0.2f);

static float gfSpecular = 2.0f;
static float gfGlossiness = 0.8f;
static float gfSmoothness = 0.75f;
static float gfOneMinusReflectivity = 0.15f;

inline float Pow5(float x)
{
	return(x * x * x * x * x);
}

inline float3 FresnelTerm(float3 F0, float cosA)
{
	return((F0 + (1 - F0) * Pow5(1 - cosA)));
}

inline float3 FresnelLerp(float3 F0, float3 F90, half cosA)
{
	return(lerp(F0, F90, Pow5(1 - cosA)));
}

inline float PerceptualRoughnessToSpecPower(float fPerceptualRoughness)
{
	float m = fPerceptualRoughness * fPerceptualRoughness;
	float sq = max(1e-4f, m * m);
	float n = (2.0f / sq) - 2.0f;

	return(max(n, 1e-4f));
}

float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float fPerceptualRoughness)
{
	float fd90 = 0.5f + 2.0f * LdotH * LdotH * fPerceptualRoughness;
	float fLightScatter = (1.0f + (fd90 - 1.0f) * Pow5(1.0f - NdotL));
	float fViewScatter = (1.0f + (fd90 - 1.0f) * Pow5(1.0f - NdotV));

	return(fLightScatter * fViewScatter);
}

// Smith-Schlick derived for Beckmann
inline float SmithBeckmannVisibilityTerm(float NdotL, float NdotV, float fRoughness)
{
	float k = fRoughness * 0.797884560802865f; //0.797884560802865 = sqrt(2.0f / Pi)

	return(1.0f / ((NdotL * (1 - k) + k) * (NdotV * (1 - k) + k) + 1e-5f)); 
}

inline float NDFBlinnPhongNormalizedTerm(float NdotH, float fRoughnessToSpecPower)
{
	float fNormTerm = (fRoughnessToSpecPower + 2.0f) * (0.5f / 3.14159f);
	float fSpecTerm = pow(NdotH, fRoughnessToSpecPower);

	return(fNormTerm * fSpecTerm);
}

float4 PSLighting(VS_OUTPUT input) : SV_TARGET
{
    float3 gfLightDirection = float3(gfLightDirectionX, gfLightDirectionY, gfLightDirectionZ);
    float3 gf3LightColor = float3(gf3LightColorX, gf3LightColorY, gf3LightColorZ);
	
	// Normalize normal
    float3 N = normalize(input.normalW);

    // ���� ����(��������, -���̸� '���� ���� ����')
    float3 L = normalize(-gfLightDirection);

    // ī�޶� ����
    float3 V = normalize(gf3CameraPosition - input.positionW);

    // Half vector (Blinn-Phong)
    float3 H = normalize(L + V);

    // ���� ���
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));

    // �ں��Ʈ ��
    float3 ambient = gf3AmbientLightColor * gf3ObjectColor;

    // ��ǻ�� ��
    float3 diffuse = gf3LightColor * gf3ObjectColor * NdotL;

    // ����ŧ�� ��
    float shininess = 4.0f; // ���̶���Ʈ ���� (���� ��, ���� ����)
    float3 specular = gf3SpecularColor * pow(NdotH, shininess);
	
    // �ջ�
    float3 finalColor = ambient + diffuse + specular;
	
    return float4(finalColor, 1.0f);

}

float4 PSTerrainLighting(VS_OUTPUT input) : SV_TARGET
{
    float3 gfLightDirection = float3(gfLightDirectionX, gfLightDirectionY, gfLightDirectionZ);
    float3 gf3LightColor = float3(gf3LightColorX, gf3LightColorY, gf3LightColorZ);
	
	// Normalize normal
    float3 N = normalize(input.normalW);

    // ���� ����(��������, -���̸� '���� ���� ����')
    float3 L = normalize(-gfLightDirection);

    // ī�޶� ����
    float3 V = normalize(gf3CameraPosition - input.positionW);

    // Half vector (Blinn-Phong)
    float3 H = normalize(L + V);

    // ���� ���
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));

    // �ں��Ʈ ��
    float3 ambient = gf3AmbientLightColor * gf3ObjectColor;

    // ��ǻ�� ��
    float3 diffuse = gf3LightColor * gf3ObjectColor * NdotL;
    float ymin = -20.0f; // �ٴ� ����
    float ymax = 31.0f; // �ִ� ����

    float yFactor = saturate((input.positionW.y - ymin) / (ymax - ymin));
    diffuse *= lerp(0.2, 2.0, yFactor);

    // ����ŧ�� ��
    float shininess = 4.0f; // ���̶���Ʈ ���� (���� ��, ���� ����)
    float3 specular = gf3SpecularColor * pow(NdotH, shininess);
	
    // �ջ�
    float3 finalColor = ambient + diffuse + specular;
	
    return float4(finalColor, 1.0f);

}