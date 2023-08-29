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
	addNewConstBufferResc(
		Utility::genRandomInt(10), Utility::genRandomInt(10), Utility::genRandomInt(10),
		Utility::genRandomInt(2), Utility::genRandomInt(2), Utility::genRandomInt(2),
		Utility::genRandomInt(360), Utility::genRandomInt(360), Utility::genRandomInt(360),
		camView, camProjection
	);
}

}