#include "constbuffer.h"

#include "dxdebug.h"

namespace tre {

void ConstantBuffer::setCamConstBuffer(ID3D11Device* device, ID3D11DeviceContext* context, XMMATRIX camViewMatrix, XMMATRIX viewProjection, const std::vector<XMMATRIX>& lightViewProjection, const tre::Light& dirLight, int numOfPointLight) {

	D3D11_BUFFER_DESC constantBufferDescCam;
	constantBufferDescCam.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescCam.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDescCam.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDescCam.MiscFlags = 0u;
	constantBufferDescCam.ByteWidth = sizeof(constBufferShaderRescCam);
	constantBufferDescCam.StructureByteStride = 0u;

	constBufferShaderRescCam constBufferRescCam;
	constBufferRescCam.camViewMatrix = camViewMatrix;
	constBufferRescCam.viewProjection = viewProjection;
	std::copy(lightViewProjection.begin(), lightViewProjection.end(), constBufferRescCam.lightViewProjection);
	constBufferRescCam.light = dirLight;
	constBufferRescCam.numOfPointLight = numOfPointLight;

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

}