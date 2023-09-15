#include "renderer.h"
#include "mesh.h"
#include "device.h"
#include "dxdebug.h"
#include "maths.h"

namespace tre {

void Renderer::draw(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RasterizerState* rasterizerState, const std::vector<Object>& objQ) {

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

		//Config and set const buffer
		tre::ConstantBuffer::setObjConstBuffer(
			device, context,
			tre::Maths::createTransformationMatrix(currObj.objScale, currObj.objRotation, currObj.objPos),
			currObj.objColor,
			currObj.isObjWithTexture,
			currObj.isObjWithNormalMap
		);

		// set normal map
		if (currObj.isObjWithNormalMap) {
			context->PSSetShaderResources(1, 1, currObj.pObjNormalMap->pShaderResView.GetAddressOf());
		}

		context->DrawIndexed(currObj.pObjMesh->indexSize, 0, 0);
	}
}

void Renderer::debugDraw(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RasterizerState* rasterizerState, std::vector<Object>& objQ, Mesh& mesh, BoundVolumeEnum typeOfBound) {

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

		//Config and set const buffer
		tre::ConstantBuffer::setObjConstBuffer(
			device, context,
			transformM,
			currObj.objColor,
			currObj.isObjWithTexture,
			currObj.isObjWithNormalMap
		);

		// set normal map
		if (currObj.isObjWithNormalMap) {
			context->PSSetShaderResources(1, 1, currObj.pObjNormalMap->pShaderResView.GetAddressOf());
		}

		context->DrawIndexed(mesh.indexSize, 0, 0);
	}
}

}