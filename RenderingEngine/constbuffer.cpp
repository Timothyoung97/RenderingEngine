#include "constbuffer.h"

#include "dxdebug.h"
#include "window.h"

namespace tre {

void ConstantBuffer::setCamConstBuffer(
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
	constantBufferDescCam.ByteWidth = sizeof(constBufferShaderRescCam);
	constantBufferDescCam.StructureByteStride = 0u;

	constBufferShaderRescCam constBufferRescCam;
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
}

void ConstantBuffer::setObjConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX transformationLocal, XMFLOAT4 color, UINT isWithTexture, UINT hasNormalMap) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDescModel.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(constBufferShaderRescModel);
	constantBufferDescModel.StructureByteStride = 0u;

	constBufferShaderRescModel constBufferRescModel;
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
}

void ConstantBuffer::setLightingVolumeConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, int currPtLightIdx) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDescModel.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(constBufferDeferredLightingVolume);
	constantBufferDescModel.StructureByteStride = 0u;

	constBufferDeferredLightingVolume constBufferLightingVolume;
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
}

void ConstantBuffer::setSSAOKernalConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDescModel.CPUAccessFlags = 0u;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(constBufferSSAOKernal);
	constantBufferDescModel.StructureByteStride = 0u;

	constBufferSSAOKernal constBufferSSAOKernal;
	std::copy(kernalSamples.begin(), kernalSamples.end(), constBufferSSAOKernal.kernalSamples);
	constBufferSSAOKernal.sampleRadius = sampleRadius;

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &constBufferSSAOKernal;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	// to set const shader for ssao
	context->PSSetConstantBuffers(3u, 1u, &pConstBuffer);
}

void ConstantBuffer::setHDRConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, float middleGrey) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDescModel.CPUAccessFlags = 0u;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(constBufferHDR);
	constantBufferDescModel.StructureByteStride = 0u;

	constBufferHDR constBufferHDR;
	constBufferHDR.middleGrey = middleGrey;

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &constBufferHDR;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	// to set const shader for ssao
	context->PSSetConstantBuffers(4u, 1u, &pConstBuffer);
}

void ConstantBuffer::setLuminaceConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMFLOAT2 luminance, float timeCoeff) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDescModel.CPUAccessFlags = 0u;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(constBufferLuminance);
	constantBufferDescModel.StructureByteStride = 0u;

	constBufferLuminance constBufferLumin;
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
}

void ConstantBuffer::setLightViewProjectionConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX viewProjection) {
	D3D11_BUFFER_DESC constantBufferDescModel;
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDescModel.CPUAccessFlags = 0u;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(constBufferDirLightViewProjection);
	constantBufferDescModel.StructureByteStride = 0u;

	constBufferDirLightViewProjection constBufferDirLightViewProj;
	constBufferDirLightViewProj.csmViewProjection = viewProjection;

	//map to data to subresouce
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &constBufferDirLightViewProj;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescModel, &csd, &pConstBuffer
	));

	context->VSSetConstantBuffers(0u, 1u, &pConstBuffer);
}

}