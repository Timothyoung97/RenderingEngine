#include "constbuffer.h"

#include "dxdebug.h"
#include "window.h"

namespace tre {

// Creates an empty constant buffer with the specified size (in bytes)
ID3D11Buffer* ConstantBuffer::createConstBuffer(ID3D11Device* pDevice, UINT sizeOfBuffer) {
	D3D11_BUFFER_DESC constBufferDesc;
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.MiscFlags = 0u;
	constBufferDesc.ByteWidth = sizeOfBuffer;
	constBufferDesc.StructureByteStride = 0u;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(pDevice->CreateBuffer(
		&constBufferDesc, nullptr, &pConstBuffer
	));

	return pConstBuffer;
}

// To update a constant buffer with data from the CPU 
void ConstantBuffer::updateConstBufferData(ID3D11DeviceContext* pContext, ID3D11Buffer* pConstBuffer, void* pConstBufferInfoStruct, UINT sizeOfConstBufferInfo) {
	
	// Disable GPU access to constant buffer data
	D3D11_MAPPED_SUBRESOURCE currData;
	CHECK_DX_ERROR(pContext->Map(pConstBuffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &currData));

	// update data
	memcpy(currData.pData, pConstBufferInfoStruct, sizeOfConstBufferInfo);

	// Reenable GPU access to constant buffer data
	pContext->Unmap(pConstBuffer, 0u);
}

/////////////// Struct Creation Functions ///////////////

GlobalInfoStruct ConstantBuffer::createGlobalInfoStruct(
	const XMVECTOR& camPos,
	const XMMATRIX& viewProjection,
	const std::vector<XMMATRIX>& lightViewProjection,
	const XMFLOAT4& planeIntervals,
	const tre::Light& dirLight,
	int numOfPointLight,
	const XMFLOAT2& shadowMapDimension,
	int csmDebugSwitch,
	int ssaoSwtich
) {
	GlobalInfoStruct GlobalInfoStruct;
	GlobalInfoStruct.viewportDimension = XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT);
	XMFLOAT4 camPosF;
	XMStoreFloat4(&camPosF, camPos);
	GlobalInfoStruct.camPos = camPosF;
	GlobalInfoStruct.viewProjection = viewProjection;
	GlobalInfoStruct.invViewProjection = XMMatrixInverse(nullptr, viewProjection);
	std::copy(lightViewProjection.begin(), lightViewProjection.end(), GlobalInfoStruct.lightViewProjection);
	GlobalInfoStruct.planeIntervals = planeIntervals;
	GlobalInfoStruct.light = dirLight;
	GlobalInfoStruct.numOfPointLight = (UINT)numOfPointLight;
	GlobalInfoStruct.csmDebugSwitch = csmDebugSwitch;
	GlobalInfoStruct.shadowMapDimension = shadowMapDimension;
	GlobalInfoStruct.ssaoSwitch = ssaoSwtich;

	return GlobalInfoStruct;
};

CSMViewProjectionStruct ConstantBuffer::createCSMViewProjectionStruct(const XMMATRIX& viewProjection) {
	CSMViewProjectionStruct csmViewProj;
	csmViewProj.csmViewProjection = viewProjection;
	return csmViewProj;
}

ModelInfoStruct ConstantBuffer::createModelInfoStruct(const XMMATRIX& transformationLocal, const XMFLOAT4& color, UINT isWithTexture, UINT hasNormalMap) {
	ModelInfoStruct modelInfoStruct;
	modelInfoStruct.transformationLocal = transformationLocal;

	XMMATRIX normalMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, modelInfoStruct.transformationLocal));
	XMFLOAT3X3 normalFloat3x3;
	XMStoreFloat3x3(&normalFloat3x3, normalMatrix);
	modelInfoStruct.normalMatrix = XMLoadFloat3x3(&normalFloat3x3);

	modelInfoStruct.isWithTexture = isWithTexture;
	modelInfoStruct.hasNormalMap = hasNormalMap;
	modelInfoStruct.color = color;

	return modelInfoStruct;
}

PointLightInfoStruct ConstantBuffer::createPointLightInfoStruct(int currPtLightIdx) {
	PointLightInfoStruct ptLightInfoStruct;
	ptLightInfoStruct.currPointLightIdx = (UINT)currPtLightIdx;

	return ptLightInfoStruct;
};

// deprecated //
ID3D11Buffer* ConstantBuffer::setCamConstBuffer(
	ID3D11Device* device, ID3D11DeviceContext* context, 
	XMVECTOR camPos, 
	XMMATRIX viewProjection, 
	const std::vector<XMMATRIX>& lightViewProjection, 
	XMFLOAT4 planeIntervals, 
	const tre::Light& dirLight, 
	int numOfPointLight, 
	XMFLOAT2 shadowMapDimension, 
	int csmDebugSwitch,
	int ssaoSwtich) {

	D3D11_BUFFER_DESC constantBufferDescCam;
	constantBufferDescCam.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescCam.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDescCam.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDescCam.MiscFlags = 0u;
	constantBufferDescCam.ByteWidth = sizeof(GlobalInfoStruct);
	constantBufferDescCam.StructureByteStride = 0u;

	GlobalInfoStruct constBufferRescCam;
	constBufferRescCam.viewportDimension = XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT);
	XMFLOAT4 camPosF;
	XMStoreFloat4(&camPosF, camPos);
	constBufferRescCam.camPos = camPosF;
	constBufferRescCam.viewProjection = viewProjection;
	constBufferRescCam.invViewProjection = XMMatrixInverse(nullptr, viewProjection);
	std::copy(lightViewProjection.begin(), lightViewProjection.end(), constBufferRescCam.lightViewProjection);
	constBufferRescCam.planeIntervals = planeIntervals;
	constBufferRescCam.light = dirLight;
	constBufferRescCam.numOfPointLight = (UINT) numOfPointLight;
	constBufferRescCam.csmDebugSwitch = csmDebugSwitch;
	constBufferRescCam.shadowMapDimension = shadowMapDimension;
	constBufferRescCam.ssaoSwitch = ssaoSwtich;

	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &constBufferRescCam;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescCam, &csd, &pConstBuffer
	));

	context->VSSetConstantBuffers(0u, 1u, &pConstBuffer);
	context->PSSetConstantBuffers(0u, 1u, &pConstBuffer);
	context->CSSetConstantBuffers(0u, 1u, &pConstBuffer);

	return pConstBuffer;
}

ID3D11Buffer* ConstantBuffer::setObjConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX transformationLocal, XMFLOAT4 color, UINT isWithTexture, UINT hasNormalMap) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDescModel.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(ModelInfoStruct);
	constantBufferDescModel.StructureByteStride = 0u;

	ModelInfoStruct constBufferRescModel;
	constBufferRescModel.transformationLocal = transformationLocal;

	XMMATRIX normalMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, constBufferRescModel.transformationLocal));
	XMFLOAT3X3 normalFloat3x3;
	XMStoreFloat3x3(&normalFloat3x3, normalMatrix);
	constBufferRescModel.normalMatrix = XMLoadFloat3x3(&normalFloat3x3);

	constBufferRescModel.isWithTexture = isWithTexture;
	constBufferRescModel.hasNormalMap = hasNormalMap;
	constBufferRescModel.color = color;

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &constBufferRescModel;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	//Set const buffer for pixel and vertex shader
	context->VSSetConstantBuffers(1u, 1u, &pConstBuffer);
	context->PSSetConstantBuffers(1u, 1u, &pConstBuffer);

	return pConstBuffer;
}

ID3D11Buffer* ConstantBuffer::setLightingVolumeConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, int currPtLightIdx) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDescModel.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(PointLightInfoStruct);
	constantBufferDescModel.StructureByteStride = 0u;

	PointLightInfoStruct constBufferLightingVolume;
	constBufferLightingVolume.currPointLightIdx = (UINT) currPtLightIdx;

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &constBufferLightingVolume;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	//Set const buffer for pixel shader
	context->PSSetConstantBuffers(2u, 1u, &pConstBuffer);

	return pConstBuffer;
}

ID3D11Buffer* ConstantBuffer::setSSAOKernalConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDescModel.CPUAccessFlags = 0u;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(SSAOKernalStruct);
	constantBufferDescModel.StructureByteStride = 0u;

	SSAOKernalStruct SSAOKernalStruct;
	std::copy(kernalSamples.begin(), kernalSamples.end(), SSAOKernalStruct.kernalSamples);
	SSAOKernalStruct.sampleRadius = sampleRadius;

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &SSAOKernalStruct;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	// to set const shader for ssao
	context->PSSetConstantBuffers(3u, 1u, &pConstBuffer);

	return pConstBuffer;
}

ID3D11Buffer* ConstantBuffer::setHDRConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, float middleGrey) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDescModel.CPUAccessFlags = 0u;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(HDRStruct);
	constantBufferDescModel.StructureByteStride = 0u;

	HDRStruct HDRStruct;
	HDRStruct.middleGrey = middleGrey;

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &HDRStruct;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	// to set const shader for ssao
	context->PSSetConstantBuffers(4u, 1u, &pConstBuffer);
	
	return pConstBuffer;
}

ID3D11Buffer* ConstantBuffer::setLuminaceConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMFLOAT2 luminance, float timeCoeff) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDescModel.CPUAccessFlags = 0u;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(LuminanceStruct);
	constantBufferDescModel.StructureByteStride = 0u;

	LuminanceStruct constBufferLumin;
	constBufferLumin.luminance = luminance;
	constBufferLumin.timeCoeff = timeCoeff;
	constBufferLumin.numPixel = SCREEN_HEIGHT * SCREEN_WIDTH;
	constBufferLumin.viewportDimension = XMINT2(SCREEN_WIDTH, SCREEN_HEIGHT);

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &constBufferLumin;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	context->CSSetConstantBuffers(0u, 1u, &pConstBuffer);
	
	return pConstBuffer;
}

ID3D11Buffer* ConstantBuffer::setLightViewProjectionConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX viewProjection) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDescModel.CPUAccessFlags = 0u;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(CSMViewProjectionStruct);
	constantBufferDescModel.StructureByteStride = 0u;

	CSMViewProjectionStruct constBufferDirLightViewProj;
	constBufferDirLightViewProj.csmViewProjection = viewProjection;

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &constBufferDirLightViewProj;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	context->VSSetConstantBuffers(0u, 1u, &pConstBuffer);

	return pConstBuffer;
}

ID3D11Buffer* ConstantBuffer::setBatchInfoConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, int batchOffset) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDescModel.CPUAccessFlags = 0u;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(CSMViewProjectionStruct);
	constantBufferDescModel.StructureByteStride = 0u;

	BatchInfoStruct BatchInfoStruct;
	BatchInfoStruct.batchOffset = (UINT)batchOffset;

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &BatchInfoStruct;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	context->VSSetConstantBuffers(1u, 1u, &pConstBuffer);

	return pConstBuffer;
}


}