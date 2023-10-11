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
	int csmDebugSwitch) {

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
	constBufferRescCam.numOfPointLight = numOfPointLight;
	constBufferRescCam.csmDebugSwitch = csmDebugSwitch;
	constBufferRescCam.shadowMapDimension = shadowMapDimension;

	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &constBufferRescCam;

	ID3D11Buffer* pConstBuffer;

	CHECK_DX_ERROR(device->CreateBuffer(
		&constantBufferDescCam, &csd, &pConstBuffer
	));

	context->VSSetConstantBuffers(0u, 1u, &pConstBuffer);
	context->PSSetConstantBuffers(0u, 1u, &pConstBuffer);
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

}