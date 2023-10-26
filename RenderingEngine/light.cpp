#include "light.h"
#include "dxdebug.h"
#include "utility.h"
#include "object.h"

namespace tre {

void LightResource::create(ID3D11Device* device, ID3D11DeviceContext* context) {
	
	_device = device;
	_context = context;

	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	computeShaderPtLightMovement.create(basePathWstr + L"shaders\\bin\\compute_shader_ptLight_movement.bin", _device);
	
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

	// Create buffer on CPU side
	D3D11_BUFFER_DESC lightBufferDescCPU;
	lightBufferDescCPU.BindFlags = 0;
	lightBufferDescCPU.Usage = D3D11_USAGE_STAGING;
	lightBufferDescCPU.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	lightBufferDescCPU.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	lightBufferDescCPU.ByteWidth = static_cast<UINT>(sizeof(tre::PointLight)) * maxPointLightNum;
	lightBufferDescCPU.StructureByteStride = static_cast<UINT>(sizeof(tre::PointLight));

	ID3D11Buffer* pLightBufferCPU0;
	CHECK_DX_ERROR(_device->CreateBuffer(
		&lightBufferDescCPU, NULL, &pLightBufferCPU0
	));
	doubleBuffer[0] = pLightBufferCPU0;

	ID3D11Buffer* pLightBufferCPU1;
	CHECK_DX_ERROR(_device->CreateBuffer(
		&lightBufferDescCPU, NULL, &pLightBufferCPU1
	));
	doubleBuffer[1] = pLightBufferCPU1;
}

void LightResource::updatePixelShaderBuffer() {
	// update GPU on buffer
	D3D11_BUFFER_SRV lightBufferSRV;
	lightBufferSRV.NumElements = numOfLights;
	lightBufferSRV.FirstElement = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC lightShaderResc;
	lightShaderResc.Format = DXGI_FORMAT_UNKNOWN;
	lightShaderResc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	lightShaderResc.Buffer = lightBufferSRV;

	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		pLightBufferGPU.Get(), &lightShaderResc, pLightShaderRescView.GetAddressOf()
	));
}

void LightResource::updateComputeShaderBuffer(PointLight newPointLight) {

	// Create buffer on CPU side to update new light resource
	D3D11_BUFFER_DESC lightBufferDescCPU;
	lightBufferDescCPU.BindFlags = 0;
	lightBufferDescCPU.Usage = D3D11_USAGE_STAGING;
	lightBufferDescCPU.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDescCPU.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	lightBufferDescCPU.ByteWidth = static_cast<UINT>(sizeof(tre::PointLight));
	lightBufferDescCPU.StructureByteStride = static_cast<UINT>(sizeof(tre::PointLight));

	D3D11_SUBRESOURCE_DATA lightData = {};
	lightData.pSysMem = &newPointLight;

	ID3D11Buffer* pLightBufferCPU;

	CHECK_DX_ERROR(_device->CreateBuffer(
		&lightBufferDescCPU, &lightData, &pLightBufferCPU
	));

	// Copy subresource from CPU to GPU
	_context->CopySubresourceRegion(pLightBufferGPU.Get(), 0, (numOfLights - 1) * static_cast<UINT>(sizeof(tre::PointLight)), 0, 0, pLightBufferCPU, 0, NULL);

	// update GPU on buffer
	D3D11_BUFFER_UAV lightBufferUAV;
	lightBufferUAV.NumElements = numOfLights;
	lightBufferUAV.FirstElement = 0;
	lightBufferUAV.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;

	D3D11_UNORDERED_ACCESS_VIEW_DESC lightUnorderAccessViewDesc;
	lightUnorderAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	lightUnorderAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	lightUnorderAccessViewDesc.Buffer = lightBufferUAV;

	CHECK_DX_ERROR(_device->CreateUnorderedAccessView(
		pLightBufferGPU.Get(), &lightUnorderAccessViewDesc, pLightUnorderedAccessView.GetAddressOf()
	));
}

void LightResource::addRandPointLight() {

	if (numOfLights < maxPointLightNum) {
		addPointLight(
			XMFLOAT3(tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20)), 
			XMFLOAT3(1.f, .14f, .07f),
			XMFLOAT4(tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f), tre::Utility::getRandomFloat(1.0f)),
			XMFLOAT2(tre::Utility::getRandomFloatRange(0.f, 360.f), tre::Utility::getRandomFloatRange(0.f, 360.f))
		);
	}
}

PointLight LightResource::addPointLight(XMFLOAT3 pos, XMFLOAT3 att, XMFLOAT4 diffuse, XMFLOAT2 yawPitch) {

	PointLight newPtLight;

	newPtLight.pos = pos;
	newPtLight.att = att;
	newPtLight.diffuse = diffuse;
	newPtLight.pad = .0f;
	
	float maxIntensity = std::max(std::max(diffuse.x, diffuse.y), diffuse.z);

	newPtLight.range = (-att.y + sqrtf(att.y * att.y - 4 * att.z * (att.x - maxIntensity * (1 / defaultBrightnessThreshold)))) / (2 * att.z);

	newPtLight.yawPitch = yawPitch;

	numOfLights++;
	updateComputeShaderBuffer(newPtLight);
	updatePixelShaderBuffer();

	return newPtLight;
}

void LightResource::updatePtLightCPU() {

	_context->CopyResource(doubleBuffer[writeIndex], pLightBufferGPU.Get());

	D3D11_MAPPED_SUBRESOURCE data;
	CHECK_DX_ERROR(_context->Map(doubleBuffer[readIndex], 0, D3D11_MAP_READ, 0u, &data));

	readOnlyPointLightQ.clear();

	PointLight* pPtLight = (PointLight*)data.pData;

	for (int i = 0; i < numOfLights; i++) {
		readOnlyPointLightQ.push_back(pPtLight[i]);
	}

	_context->Unmap(doubleBuffer[readIndex], 0);

	readIndex ^= 1;
	writeIndex ^= 1;
}

void LightResource::dispatch() {
	// using compute shader update lights
	_context->CSSetShader(computeShaderPtLightMovement.pShader.Get(), NULL, 0u);
	_context->CSSetUnorderedAccessViews(0, 1, pLightUnorderedAccessView.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Compute Shader");
		_context->Dispatch(tre::Maths::divideAndRoundUp(numOfLights, 4u), 1u, 1u);
	}
	_context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}
}