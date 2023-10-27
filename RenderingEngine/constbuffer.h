#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <vector>

#include <light.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace tre {

struct constBufferShaderRescCam {
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

struct constBufferDirLightViewProjection {
	XMMATRIX csmViewProjection;
};

struct constBufferShaderRescModel {
	XMMATRIX transformationLocal;
	XMMATRIX normalMatrix;
	XMFLOAT4 color;
	UINT isWithTexture;
	UINT hasNormalMap;
};

struct constBufferDeferredLightingVolume {
	UINT currPointLightIdx;
	XMFLOAT3 pad;
};

struct constBufferSSAOKernal {
	XMFLOAT4 kernalSamples[64];
	float sampleRadius;
	XMFLOAT3 pad;
};

struct constBufferHDR {
	float middleGrey;
	XMFLOAT3 pad;
};

struct constBufferLuminance {
	XMFLOAT2 luminance;
	float timeCoeff;
	int numPixel;
	XMINT2 viewportDimension;
	XMINT2 pad;
};

struct constBufferBatchInformation {
	UINT batchOffset;
};

class ConstantBuffer {
public:
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