#include "renderer.h"
#include "mesh.h"
#include "device.h"
#include "dxdebug.h"

namespace tre {

void Renderer::draw(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RasterizerState* rasterizerState, tre::ConstantBuffer& cb, const std::vector<Object>& objQ) {

	context->RSSetState(rasterizerState);
	
	for (int i = 0; i < objQ.size(); i++) {

		const tre::Object& currObj = objQ[i];

		//Set vertex buffer
		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, currObj.pObjMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		context->IASetIndexBuffer(currObj.pObjMesh->pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//set shader resc view and sampler
		context->PSSetShaderResources(0, 1, currObj.pObjTexture->pShaderResView.GetAddressOf());

		//Config const buffer
		cb.constBufferRescModel.transformationLocal = XMMatrixMultiply(
			XMMatrixScaling(currObj.objScale.x, currObj.objScale.y, currObj.objScale.z),
			XMMatrixMultiply(
				XMMatrixRotationRollPitchYaw(XMConvertToRadians(currObj.objRotation.x), XMConvertToRadians(currObj.objRotation.y), XMConvertToRadians(currObj.objRotation.z)),
				XMMatrixTranslation(currObj.objPos.x, currObj.objPos.y, currObj.objPos.z)
			)
		);
		
		XMMATRIX normalMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cb.constBufferRescModel.transformationLocal));

		XMFLOAT3X3 normalFloat3x3;
		XMStoreFloat3x3(&normalFloat3x3, normalMatrix);

		cb.constBufferRescModel.normalMatrix = XMLoadFloat3x3(&normalFloat3x3);

		cb.constBufferRescModel.isWithTexture = currObj.isObjWithTexture;
		cb.constBufferRescModel.hasNormalMap = currObj.isObjWithNormalMap;
		cb.constBufferRescModel.color = currObj.objColor;

		// set normal map
		if (cb.constBufferRescModel.hasNormalMap) {
			context->PSSetShaderResources(1, 1, currObj.pObjNormalMap->pShaderResView.GetAddressOf());
		}

		//map to data to subresouce
		cb.csd.pSysMem = &cb.constBufferRescModel;

		CHECK_DX_ERROR(device->CreateBuffer(
			&cb.constantBufferDescModel, &cb.csd, cb.pConstBuffer.GetAddressOf()
		));

		//Set const buffer for pixel and vertex shader
		context->VSSetConstantBuffers(1u, 1u, cb.pConstBuffer.GetAddressOf());
		context->PSSetConstantBuffers(1u, 1u, cb.pConstBuffer.GetAddressOf());

		context->DrawIndexed(currObj.pObjMesh->indexSize, 0, 0);
	}
}

void Renderer::debugDraw(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RasterizerState* rasterizerState, tre::ConstantBuffer& cb, const std::vector<Object>& objQ, const Mesh& sphere, BoundVolumeEnum typeOfBound) {

	context->RSSetState(rasterizerState);

	for (int i = 0; i < objQ.size(); i++) {

		const tre::Object& currObj = objQ[i];

		//Set vertex buffer
		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, sphere.pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		context->IASetIndexBuffer(sphere.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//set shader resc view and sampler
		context->PSSetShaderResources(0, 1, currObj.pObjTexture->pShaderResView.GetAddressOf());

		//check for type of bounding sphere
		tre::BoundingVolume currBV;
		XMFLOAT3 boundingVolScale(.0f, .0f, .0f);
		switch (typeOfBound) {
		case RitterBoundingSphere:
			currBV = currObj.ritterBs;
			boundingVolScale = XMFLOAT3(currObj.ritterBs.radius, currObj.ritterBs.radius, currObj.ritterBs.radius);
			break;
		case NaiveBoundingSphere:
			currBV = currObj.naiveBs;
			boundingVolScale = XMFLOAT3(currObj.ritterBs.radius, currObj.ritterBs.radius, currObj.ritterBs.radius);
			break;
		case AABBBoundingBox:
			currBV = currObj.aabb;
			boundingVolScale = currObj.aabb.halfExtent;
			break;
		}

		XMMATRIX transformation = XMMatrixMultiply(
			XMMatrixScaling(currObj.objScale.x, currObj.objScale.y, currObj.objScale.z),
			XMMatrixMultiply(
				XMMatrixRotationRollPitchYaw(XMConvertToRadians(currObj.objRotation.x), XMConvertToRadians(currObj.objRotation.y), XMConvertToRadians(currObj.objRotation.z)),
				XMMatrixTranslation(
					currObj.objPos.x,
					currObj.objPos.y,
					currObj.objPos.z)
			)
		);

		XMVECTOR localCenterV = { currBV.center.x, currBV.center.y, currBV.center.z, 1 };

		localCenterV = XMVector4Transform(localCenterV, transformation);

		XMFLOAT4 newCenter;
		XMStoreFloat4(&newCenter, localCenterV);

		XMMATRIX transformM = XMMatrixMultiply(
			XMMatrixScaling(currObj.objScale.x * boundingVolScale.x / unitLength, currObj.objScale.y * boundingVolScale.y / unitLength, currObj.objScale.z * boundingVolScale.z / unitLength),
			XMMatrixMultiply(
				XMMatrixRotationRollPitchYaw(.0f, .0f, .0f),
				XMMatrixTranslation(
					newCenter.x,
					newCenter.y,
					newCenter.z
				)
			)
		);

		//Config const buffer
		cb.constBufferRescModel.transformationLocal = transformM;

		XMMATRIX normalMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cb.constBufferRescModel.transformationLocal));

		XMFLOAT3X3 normalFloat3x3;
		XMStoreFloat3x3(&normalFloat3x3, normalMatrix);

		cb.constBufferRescModel.normalMatrix = XMLoadFloat3x3(&normalFloat3x3);

		cb.constBufferRescModel.isWithTexture = currObj.isObjWithTexture;
		cb.constBufferRescModel.hasNormalMap = currObj.isObjWithNormalMap;
		cb.constBufferRescModel.color = currObj.objColor;

		// set normal map
		if (cb.constBufferRescModel.hasNormalMap) {
			context->PSSetShaderResources(1, 1, currObj.pObjNormalMap->pShaderResView.GetAddressOf());
		}

		//map to data to subresouce
		cb.csd.pSysMem = &cb.constBufferRescModel;

		CHECK_DX_ERROR(device->CreateBuffer(
			&cb.constantBufferDescModel, &cb.csd, cb.pConstBuffer.GetAddressOf()
		));

		//Set const buffer for pixel and vertex shader
		context->VSSetConstantBuffers(1u, 1u, cb.pConstBuffer.GetAddressOf());
		context->PSSetConstantBuffers(1u, 1u, cb.pConstBuffer.GetAddressOf());

		context->DrawIndexed(sphere.indexSize, 0, 0);
	}
}

}