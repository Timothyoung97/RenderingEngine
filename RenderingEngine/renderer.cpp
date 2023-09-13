#include "renderer.h"
#include "mesh.h"
#include "device.h"
#include "dxdebug.h"
#include "maths.h"

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
		cb.constBufferRescModel.transformationLocal = tre::Maths::createTransformationMatrix(currObj.objScale, currObj.objRotation, currObj.objPos);
		
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

void Renderer::debugDraw(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RasterizerState* rasterizerState, tre::ConstantBuffer& cb, std::vector<Object>& objQ, Mesh& mesh, BoundVolumeEnum typeOfBound) {

	context->RSSetState(rasterizerState);

	for (int i = 0; i < objQ.size(); i++) {

		tre::Object& currObj = objQ[i];

		//Set vertex buffer
		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, mesh.pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		context->IASetIndexBuffer(mesh.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//set shader resc view and sampler
		context->PSSetShaderResources(0, 1, currObj.pObjTexture->pShaderResView.GetAddressOf());

		//Update bounding volume
		XMMATRIX transformM;
		switch (typeOfBound) {
		case RitterBoundingSphere:
			transformM = tre::BoundingVolume::updateBoundingSphere(currObj.pObjMesh->ritterSphere, currObj.ritterBs, currObj.objScale, currObj.objRotation, currObj.objPos);
			break;

		case NaiveBoundingSphere:
			transformM = tre::BoundingVolume::updateBoundingSphere(currObj.pObjMesh->naiveSphere, currObj.naiveBs, currObj.objScale, currObj.objRotation, currObj.objPos);
			break;

		case AABBBoundingBox:
			transformM = tre::BoundingVolume::updateAABB(currObj.pObjMesh->aabb, currObj.aabb, currObj.objScale, currObj.objRotation, currObj.objPos);
			break;
		}

		//Set const buffer
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

		context->DrawIndexed(mesh.indexSize, 0, 0);
	}
}

}