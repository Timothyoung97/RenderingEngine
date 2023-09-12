#include "renderer.h"
#include "mesh.h"
#include "device.h"
#include "dxdebug.h"
#include "matrix.h"

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
		cb.constBufferRescModel.transformationLocal = tre::Matrix::createTransformationMatrix(currObj.objScale, currObj.objRotation, currObj.objPos);
		
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
		BoundingSphere currBS;
		AABB aabb;
		XMFLOAT3 boundingVolScale(.0f, .0f, .0f);
		switch (typeOfBound) {
		case RitterBoundingSphere:
			currBS = currObj.ritterBs;
			boundingVolScale = XMFLOAT3(currObj.ritterBs.radius, currObj.ritterBs.radius, currObj.ritterBs.radius);
			break;
		case NaiveBoundingSphere:
			currBS = currObj.naiveBs;
			boundingVolScale = XMFLOAT3(currObj.ritterBs.radius, currObj.ritterBs.radius, currObj.ritterBs.radius);
			break;
		case AABBBoundingBox:
			aabb = currObj.aabb;
			boundingVolScale = aabb.halfExtent;
		}

		XMMATRIX transformM;

		if (typeOfBound == AABBBoundingBox) {
			
			// model transformation
			XMMATRIX transformation = tre::Matrix::createTransformationMatrix(currObj.objScale, currObj.objRotation, currObj.objPos);

			// update center
			XMVECTOR localCenterV = { aabb.center.x, aabb.center.y, aabb.center.z, 1 };

			localCenterV = XMVector4Transform(localCenterV, transformation);

			// store new center
			XMFLOAT4 newCenter;
			XMStoreFloat4(&newCenter, localCenterV);

			// obtain original up, right and forward
			XMVECTOR transformRight = tre::Matrix::getMatrixNormRight(transformation);
			XMVECTOR transformUp = tre::Matrix::getMatrixNormUp(transformation);
			XMVECTOR transformForward = tre::Matrix::getMatrixNormForward(transformation);
			
			// insert code
			XMVECTOR right = transformRight * aabb.halfExtent.x;
			XMVECTOR up = transformUp * aabb.halfExtent.y;
			XMVECTOR forward = transformForward * aabb.halfExtent.z;

			XMVECTOR x = XMVECTOR{ 1.f, .0f, .0f, 0 }, y = XMVECTOR{ .0f, 1.f, .0f, 0 }, z = XMVECTOR{ .0f, .0f, 1.f, 0 };

			XMVECTOR Ii = XMVectorAbs(XMVector3Dot(x, right)) + XMVectorAbs(XMVector3Dot(x, up)) + XMVectorAbs(XMVector3Dot(x, forward));
			XMVECTOR Ij = XMVectorAbs(XMVector3Dot(y, right)) + XMVectorAbs(XMVector3Dot(y, up)) + XMVectorAbs(XMVector3Dot(y, forward));
			XMVECTOR Ik = XMVectorAbs(XMVector3Dot(z, right)) + XMVectorAbs(XMVector3Dot(z, up)) + XMVectorAbs(XMVector3Dot(z, forward));
			
			XMFLOAT3 newIi, newIj, newIk;
			XMStoreFloat3(&newIi, Ii);
			XMStoreFloat3(&newIj, Ij);
			XMStoreFloat3(&newIk, Ik);

			transformM = tre::Matrix::createTransformationMatrix(
				XMFLOAT3(currObj.objScale.x * newIi.x / unitLength, currObj.objScale.y * newIj.y / unitLength, currObj.objScale.z * newIk.z / unitLength),
				XMFLOAT3(.0f, .0f, .0f),
				XMFLOAT3(newCenter.x, newCenter.y, newCenter.z)
			);

		} else {
			// model transformation
			XMMATRIX transformation = tre::Matrix::createTransformationMatrix(currObj.objScale, currObj.objRotation, currObj.objPos);

			// update center
			XMVECTOR localCenterV = { currBS.center.x, currBS.center.y, currBS.center.z, 1 };

			localCenterV = XMVector4Transform(localCenterV, transformation);

			// store new center
			XMFLOAT4 newCenter;
			XMStoreFloat4(&newCenter, localCenterV);

			transformM = tre::Matrix::createTransformationMatrix(
				XMFLOAT3(currObj.objScale.x * boundingVolScale.x / unitLength, currObj.objScale.y * boundingVolScale.y / unitLength, currObj.objScale.z * boundingVolScale.z / unitLength),
				XMFLOAT3(.0f, .0f, .0f),
				XMFLOAT3(newCenter.x, newCenter.y, newCenter.z)
			);
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

		context->DrawIndexed(sphere.indexSize, 0, 0);
	}
}

}