#include "light.h"
#include "dxdebug.h"
#include "utility.h"
#include "object.h"

namespace tre {

LightResource::LightResource(ID3D11Device* device) {
	
	// persistent
	D3D11_BUFFER_DESC lightBufferDescGPU;
	lightBufferDescGPU.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	lightBufferDescGPU.Usage = D3D11_USAGE_DEFAULT;
	lightBufferDescGPU.CPUAccessFlags = 0u;
	lightBufferDescGPU.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	lightBufferDescGPU.ByteWidth = static_cast<UINT>(sizeof(tre::PointLight) * maxPointLightNum);
	lightBufferDescGPU.StructureByteStride = static_cast<UINT>(sizeof(tre::PointLight));

	CHECK_DX_ERROR(device->CreateBuffer(
		&lightBufferDescGPU, NULL, pLightBufferGPU.GetAddressOf()
	));
}

void LightResource::updateBuffer(ID3D11Device* device, ID3D11DeviceContext* context) {
	
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

	CHECK_DX_ERROR(device->CreateBuffer(
		&lightBufferDescCPU, &lightData, &pLightBufferCPU
	));

	// Copy subresource from CPU to GPU
	context->CopyResource( pLightBufferGPU.Get(), pLightBufferCPU );
	//context->CopySubresourceRegion(pLightBufferGPU.Get(), (UINT) pointLights.size() - 1, 0, 0, 0, pLightBufferCPU, pointLights.size() - 1, nullptr);

	// update GPU on buffer
	D3D11_BUFFER_SRV lightBufferSRV;
	lightBufferSRV.NumElements = pointLights.size();
	lightBufferSRV.FirstElement = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC lightShaderResc;
	lightShaderResc.Format = DXGI_FORMAT_UNKNOWN;
	lightShaderResc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	lightShaderResc.Buffer = lightBufferSRV;

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		pLightBufferGPU.Get(), &lightShaderResc, pLightShaderRescView.GetAddressOf()
	));
}

void LightResource::addPointLight() {

	if (pointLights.size() < maxPointLightNum) {
		PointLight newPl = {
			XMFLOAT3(.0f, .0f, .0f), 
			.0f, // pad
			XMFLOAT3(tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20)),
			tre::Utility::getRandomFloat(120.0f),
			XMFLOAT3(.0f, .2f, .0f), 
			.0f, // pad2
			XMFLOAT4(tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f)),
			XMFLOAT4(tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f))
		};

		pointLights.push_back(newPl);
	}
}
}