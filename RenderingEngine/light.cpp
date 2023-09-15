#include "light.h"
#include "dxdebug.h"

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
	lightBufferDescCPU.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	lightBufferDescCPU.Usage = D3D11_USAGE_STAGING;
	lightBufferDescCPU.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	lightBufferDescCPU.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	lightBufferDescCPU.ByteWidth = static_cast<UINT>(sizeof(tre::PointLight) * pointLights.size());
	lightBufferDescCPU.StructureByteStride = static_cast<UINT>(sizeof(tre::PointLight));
	
	D3D11_SUBRESOURCE_DATA lightData = {};
	lightData.pSysMem = pointLights.data();
	
	ID3D11Buffer* pLightBufferCPU;

	CHECK_DX_ERROR(device->CreateBuffer(
		&lightBufferDescCPU, &lightData, &pLightBufferCPU
	));

	// Copy subresource from CPU to GPU
	context->CopyResource( pLightBufferGPU.Get(), pLightBufferCPU );

	// update GPU on buffer
	D3D11_BUFFER_SRV lightBufferSRV;
	lightBufferSRV.NumElements = pointLights.size();
	lightBufferSRV.ElementOffset = 0u;
	lightBufferSRV.FirstElement = 0u;
	lightBufferSRV.ElementWidth = static_cast<UINT>(sizeof(tre::PointLight));

	D3D11_SHADER_RESOURCE_VIEW_DESC lightShaderResc;
	lightShaderResc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	lightShaderResc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	lightShaderResc.Buffer = lightBufferSRV;

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		pLightBufferGPU.Get(), &lightShaderResc, pLightShaderRescView.GetAddressOf()
	));
}
}