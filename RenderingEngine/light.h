#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

// std
#include <vector>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace tre {
struct Light {
	XMFLOAT3 direction;
	float pad;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};

struct PointLight {
	XMFLOAT3 pos;
	float range;
	XMFLOAT3 att;
	float pad;
	XMFLOAT4 diffuse;
};

class LightResource {
public:
	ComPtr<ID3D11Buffer> pLightBufferGPU;
	ComPtr<ID3D11ShaderResourceView> pLightShaderRescView;

	std::vector<PointLight> pointLights;
	int maxPointLightNum = 9;

	float defaultBrightnessThreshold = .4f / 256.f;

	void create(ID3D11Device* device);
	void updateBuffer(ID3D11Device* device, ID3D11DeviceContext* context);
	PointLight createPtLight(XMFLOAT3 pos, XMFLOAT3 att, XMFLOAT4 diffuse);
	void addPointLight();
};
}