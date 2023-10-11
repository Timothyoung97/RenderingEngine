#include "light.h"
#include "dxdebug.h"
#include "utility.h"
#include "object.h"

namespace tre {

void LightResource::create(ID3D11Device* device, ID3D11DeviceContext* context) {
	
	_device = device;
	_context = context;

	// persistent
	D3D11_BUFFER_DESC lightBufferDescGPU;
	lightBufferDescGPU.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	lightBufferDescGPU.Usage = D3D11_USAGE_DEFAULT;
	lightBufferDescGPU.CPUAccessFlags = 0u;
	lightBufferDescGPU.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	lightBufferDescGPU.ByteWidth = static_cast<UINT>(sizeof(tre::PointLight) * maxPointLightNum);
	lightBufferDescGPU.StructureByteStride = static_cast<UINT>(sizeof(tre::PointLight));

	CHECK_DX_ERROR(device->CreateBuffer(
		&lightBufferDescGPU, NULL, pLightBufferGPU.GetAddressOf()
	));

}

void LightResource::updateBufferForShaders() {
	
	// Create buffer on CPU side to update new light resource
	D3D11_BUFFER_DESC lightBufferDescCPU;
	lightBufferDescCPU.BindFlags = 0;
	lightBufferDescCPU.Usage = D3D11_USAGE_STAGING;
	lightBufferDescCPU.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	lightBufferDescCPU.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	lightBufferDescCPU.ByteWidth = static_cast<UINT>(sizeof(tre::PointLight) * maxPointLightNum);
	lightBufferDescCPU.StructureByteStride = static_cast<UINT>(sizeof(tre::PointLight));
	
	D3D11_SUBRESOURCE_DATA lightData = {};
	lightData.pSysMem = pointLights.data();
	
	ID3D11Buffer* pLightBufferCPU;

	CHECK_DX_ERROR(_device->CreateBuffer(
		&lightBufferDescCPU, &lightData, &pLightBufferCPU
	));

	// Copy subresource from CPU to GPU
	_context->CopyResource( pLightBufferGPU.Get(), pLightBufferCPU );

	// update GPU on buffer
	D3D11_BUFFER_SRV lightBufferSRV;
	lightBufferSRV.NumElements = pointLights.size();
	lightBufferSRV.FirstElement = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC lightShaderResc;
	lightShaderResc.Format = DXGI_FORMAT_UNKNOWN;
	lightShaderResc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	lightShaderResc.Buffer = lightBufferSRV;

	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		pLightBufferGPU.Get(), &lightShaderResc, pLightShaderRescView.GetAddressOf()
	));
}

void LightResource::addPointLight() {

	if (pointLights.size() < maxPointLightNum) {
		PointLight newPl = createPtLight(
				XMFLOAT3(tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20)), 
				XMFLOAT3(1.f, .14f, .07f),
				XMFLOAT4(tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f))
		);

		pointLights.push_back(newPl);
	}
}

PointLight LightResource::createPtLight(XMFLOAT3 pos, XMFLOAT3 att, XMFLOAT4 diffuse) {

	PointLight newPtLight;

	newPtLight.pos = pos;
	newPtLight.att = att;
	newPtLight.diffuse = diffuse;
	newPtLight.pad = .0f;
	
	float maxIntensity = std::max(std::max(diffuse.x, diffuse.y), diffuse.z);

	newPtLight.range = (-att.y + sqrtf(att.y * att.y - 4 * att.z * (att.x - maxIntensity * (1 / defaultBrightnessThreshold)))) / (2 * att.z);

	return newPtLight;
}
}