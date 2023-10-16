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
	float sampleRadius, sampleBias;
	XMFLOAT2 pad;
};

class ConstantBuffer {
public:
	static void setCamConstBuffer(
		ID3D11Device* device, ID3D11DeviceContext* context, 
		XMVECTOR camPos, 
		XMMATRIX viewProjection, 
		const std::vector<XMMATRIX>& lightViewProjection, 
		XMFLOAT4 planeIntervals, 
		const tre::Light& dirLight, 
		int numOfPointLight,
		XMFLOAT2 shadowMapDimension,
		int csmDebugSwitch
	);
	
	static void setObjConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX transformationLocal, XMFLOAT4 color, UINT isWithTexture, UINT hasNormalMap);

	static void setLightingVolumeConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, int currPtLightIdx);

	static void setSSAOKernalConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius, float sampleBias);
};

}