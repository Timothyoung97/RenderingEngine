#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <vector>

#include "shader.h"
#include "dxdebug.h"
#include "utility.h"
#include "object.h"

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
	XMFLOAT2 yawPitch;
	XMFLOAT2 pad2;
};

class LightResource {
public:
	
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;
	
	ComPtr<ID3D11Buffer> pLightBufferGPU;
	//ComPtr<ID3D11ShaderResourceView> pLightShaderRescView;
	//ComPtr<ID3D11UnorderedAccessView> pLightUnorderedAccessView;

	int readIndex = 0;
	int writeIndex = readIndex ^ 1;
	ComPtr<ID3D11Buffer> doubleBuffer[2];

	int numOfLights = 0;
	int maxPointLightNum = 9;

	float defaultBrightnessThreshold = .45f / 256.f;

	std::vector<PointLight> readOnlyPointLightQ;

	void create(ID3D11Device* device, ID3D11DeviceContext* context);
	void addPointLight(XMFLOAT3 pos, XMFLOAT3 att, XMFLOAT4 diffuse, XMFLOAT2 yawPitch);
	void addRandPointLight();
	void updateComputeShaderBuffer(PointLight newPointLight);
	void updatePtLightCPU();
};
}