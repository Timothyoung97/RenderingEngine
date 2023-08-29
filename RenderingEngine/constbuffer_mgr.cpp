#include "constbuffer_mgr.h"

namespace tre {

ConstantBufferManager::ConstantBufferManager() {
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0u;
	constantBufferDesc.ByteWidth = sizeof(constBufferShaderResc);
	constantBufferDesc.StructureByteStride = 0u;
}

void ConstantBufferManager::addNewConstBufferResc(
	float offsetX, float offsetY, float offsetZ, 
	float scaleX, float scaleY, float scaleZ, 
	float localYaw, float localPitch, float localRoll, 
	XMMATRIX camView, XMMATRIX camProjection) {

	// model matrix = scale -> rotate -> translate
	XMMATRIX tf_matrix = XMMatrixMultiply(
		XMMatrixScaling(scaleX, scaleY, scaleZ),
		XMMatrixMultiply(
			XMMatrixRotationRollPitchYaw(XMConvertToRadians(localPitch), XMConvertToRadians(localYaw), XMConvertToRadians(localRoll)),
			XMMatrixTranslation(offsetX, offsetY, offsetZ)
		)
	);

	constBufferShaderResc cbsr;
	cbsr.transformation = tf_matrix;
	cbsr.viewProjection = XMMatrixMultiply(camView, camProjection);

	constBufferShaderRescList.push_back(cbsr);
}

void ConstantBufferManager::addRandomConstBufferResc(XMMATRIX camView, XMMATRIX camProjection) {

	float scaleValue = Utility::getRandomInt(3);
	float offsetX = Utility::getRandomFloatRange(-5.0f, 5.0f);
	float offsetY = Utility::getRandomFloatRange(-5.0f, 5.0f);
	float offsetZ = Utility::getRandomFloatRange(-5.0f, 5.0f);

	addNewConstBufferResc(
		offsetX, offsetY, offsetZ,
		scaleValue, scaleValue, scaleValue,
		Utility::getRandomInt(360), Utility::getRandomInt(360), Utility::getRandomInt(360),
		camView, camProjection
	);
}

void ConstantBufferManager::updateCamMatrix(int idx, XMMATRIX camView, XMMATRIX camProjection) {
	assert(constBufferShaderRescList.size() > 0);
	assert(0 <= idx && idx <= constBufferShaderRescList.size() - 1);
	constBufferShaderRescList[idx].viewProjection = XMMatrixMultiply(camView, camProjection);
}

}