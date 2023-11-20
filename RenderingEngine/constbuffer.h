#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <vector>

#include <light.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace tre {

struct GlobalInfoStruct {
	XMFLOAT2 viewportDimension;
	XMFLOAT2 pad;
	XMFLOAT4 camPos;
	XMMATRIX viewProjection;
	XMMATRIX invViewProjection;
	XMMATRIX lightViewProjection[4];
	XMFLOAT4 planeIntervals;
	tre::Light light;
	UINT numOfPointLight;
	XMFLOAT2 shadowMapDimension;
	int csmDebugSwitch;
	int ssaoSwitch;
};

struct CSMViewProjectionStruct {
	XMMATRIX csmViewProjection;
};

struct ModelInfoStruct {
	XMMATRIX transformationLocal;
	XMMATRIX normalMatrix;
	XMFLOAT4 color;
	UINT isWithTexture;
	UINT hasNormalMap;
};

struct PointLightInfoStruct {
	UINT currPointLightIdx;
	XMFLOAT3 pad;
};

struct SSAOKernalStruct {
	XMFLOAT4 kernalSamples[64];
	float sampleRadius;
	XMFLOAT3 pad;
};

struct HDRStruct {
	float middleGrey;
	XMFLOAT3 pad;
};

struct LuminanceStruct {
	XMFLOAT2 luminance;
	float timeCoeff;
	int numPixel;
	XMINT2 viewportDimension;
	XMINT2 pad;
};

struct BatchInfoStruct {
	UINT batchOffset;
};

class ConstantBuffer {
public:
	
	// Creates an empty const buffer 
	static ID3D11Buffer* createConstBuffer(ID3D11Device* pDevice, UINT sizeOfBuffer);
	
	// Updates a const buffer with data from struct
	static void updateConstBufferData(ID3D11DeviceContext* pContext, ID3D11Buffer* pConstBuffer, void* pConstBufferInfo, UINT sizeOfConstBufferInfo);


	// --Struct create functions-- //

	static GlobalInfoStruct createGlobalInfoStruct(
		const XMVECTOR& camPos,
		const XMMATRIX& viewProjection,
		const std::vector<XMMATRIX>& lightViewProjection,
		const XMFLOAT4& planeIntervals,
		const tre::Light& dirLight,
		int numOfPointLight,
		const XMFLOAT2& shadowMapDimension,
		int csmDebugSwitch,
		int ssaoSwtich
	);

	static CSMViewProjectionStruct createCSMViewProjectionStruct(const XMMATRIX& viewProjection);
	static ModelInfoStruct createModelInfoStruct(const XMMATRIX& transformationLocal, const XMFLOAT4& color, UINT isWithTexture, UINT hasNormalMap);
	static PointLightInfoStruct createPointLightInfoStruct(int currPtLightIdx);
	static SSAOKernalStruct createSSAOKernalStruct(const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius);

	// --DEPRECATED-- //
	static ID3D11Buffer* setCamConstBuffer(
		ID3D11Device* device, ID3D11DeviceContext* context, 
		XMVECTOR camPos, 
		XMMATRIX viewProjection, 
		const std::vector<XMMATRIX>& lightViewProjection, 
		XMFLOAT4 planeIntervals, 
		const tre::Light& dirLight, 
		int numOfPointLight,
		XMFLOAT2 shadowMapDimension,
		int csmDebugSwitch,
		int ssaoSwtich
	);
	static ID3D11Buffer* setObjConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX transformationLocal, XMFLOAT4 color, UINT isWithTexture, UINT hasNormalMap);
	static ID3D11Buffer* setLightingVolumeConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, int currPtLightIdx);
	static ID3D11Buffer* setSSAOKernalConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius);
	static ID3D11Buffer* setHDRConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, float middleGrey);
	static ID3D11Buffer* setLuminaceConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMFLOAT2 luminance, float timeCoeff);
	static ID3D11Buffer* setLightViewProjectionConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX viewProjection);
	static ID3D11Buffer* setBatchInfoConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, int batchOffset);
};

}