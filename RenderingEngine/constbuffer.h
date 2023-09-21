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
	XMFLOAT4 camPos;
	XMMATRIX viewProjection;
	XMMATRIX lightViewProjection[4];
	tre::Light light;
	int numOfPointLight;
};

struct constBufferShaderRescModel {
	XMMATRIX transformationLocal;
	XMMATRIX normalMatrix;
	XMFLOAT4 color;
	UINT isWithTexture;
	UINT hasNormalMap;
};

class ConstantBuffer {
public:
	static void setCamConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMVECTOR camPos, XMMATRIX viewProjection, const std::vector<XMMATRIX>& lightViewProjection, const tre::Light& dirLight, int numOfPointLight);
	static void setObjConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX transformationLocal, XMFLOAT4 color, UINT isWithTexture, UINT hasNormalMap);
};

}