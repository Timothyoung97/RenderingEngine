#include "constbuffer.h"

#include "dxdebug.h"
#include "window.h"

namespace tre {

// Creates an empty constant buffer with the specified size (in bytes)
ID3D11Buffer* Buffer::createConstBuffer(ID3D11Device* pDevice, UINT sizeOfBuffer) {
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
void Buffer::updateConstBufferData(ID3D11DeviceContext* pContext, ID3D11Buffer* pConstBuffer, void* pConstBufferInfoStruct, UINT sizeOfConstBufferInfo) {
	
	// Disable GPU access to constant buffer data
	D3D11_MAPPED_SUBRESOURCE currData;
	CHECK_DX_ERROR(pContext->Map(pConstBuffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &currData));

	// update data
	memcpy(currData.pData, pConstBufferInfoStruct, sizeOfConstBufferInfo);

	// Reenable GPU access to constant buffer data
	pContext->Unmap(pConstBuffer, 0u);
}

/////////////// Struct Creation Functions ///////////////

GlobalInfoStruct CommonStructUtility::createGlobalInfoStruct(
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

ViewProjectionStruct CommonStructUtility::createViewProjectionStruct(const XMMATRIX& viewProjection) {
	ViewProjectionStruct csmViewProj;
	csmViewProj.viewProjection = viewProjection;
	return csmViewProj;
}

ModelInfoStruct CommonStructUtility::createModelInfoStruct(const XMMATRIX& transformationLocal, const XMFLOAT4& color, UINT isWithTexture, UINT hasNormalMap) {
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

PointLightInfoStruct CommonStructUtility::createPointLightInfoStruct(int currPtLightIdx) {
	PointLightInfoStruct ptLightInfoStruct;
	ptLightInfoStruct.currPointLightIdx = (UINT)currPtLightIdx;

	return ptLightInfoStruct;
};

SSAOKernalStruct CommonStructUtility::createSSAOKernalStruct(const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius) {
	SSAOKernalStruct ssaoKernalStruct;
	std::copy(kernalSamples.begin(), kernalSamples.end(), ssaoKernalStruct.kernalSamples);
	ssaoKernalStruct.sampleRadius = sampleRadius;

	return ssaoKernalStruct;
}

BatchInfoStruct CommonStructUtility::createBatchInfoStruct(int batchOffset) {
	BatchInfoStruct batchInfoStruct;
	batchInfoStruct.batchOffset = (UINT)batchOffset;
	
	return batchInfoStruct;
}


}