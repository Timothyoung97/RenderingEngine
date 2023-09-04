#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <vector>

#include <light.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct constBufferShaderRescCam {
	XMMATRIX viewProjection;
	Light light;
};

struct constBufferShaderRescModel {
	XMMATRIX transformationLocal;
	XMMATRIX normalMatrix;
	XMFLOAT4 color;
	UINT isWithTexture;
};

namespace tre {

class ConstantBuffer {
public:
	D3D11_BUFFER_DESC constantBufferDescCam;
	D3D11_BUFFER_DESC constantBufferDescModel;

	D3D11_SUBRESOURCE_DATA csd = {};
	ComPtr<ID3D11Buffer> pConstBuffer;

	constBufferShaderRescCam constBufferRescCam;
	constBufferShaderRescModel constBufferRescModel;

	ConstantBuffer();
};

}