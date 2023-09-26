#include "renderer.h"
#include "mesh.h"
#include "device.h"
#include "dxdebug.h"
#include "maths.h"

namespace tre {

Renderer::Renderer(ID3D11Device* _device, ID3D11DeviceContext* _context) : _device(_device), _context(_context) {
	_blendstate.create(_device);
}

void Renderer::configureContext(RENDER_MODE renderMode) {
	switch (renderMode)
	{
	case tre::TRANSPARENT_M:
		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		break;								
	case tre::OPAQUE_M:						
		_context->OMSetBlendState(_blendstate.opaque.Get(), NULL, 0xffffffff);
		break;								
	case tre::WIREFRAME_M:					
		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		break;								
	case tre::SHADOW_M:						
		_context->OMSetBlendState(_blendstate.transparency.Get(), NULL, 0xffffffff);
		break;
	}
}

void Renderer::draw(ID3D11RasterizerState* rasterizerState, const std::vector<Object>& objQ, RENDER_MODE renderMode) {

	configureContext(renderMode);

	_context->RSSetState(rasterizerState);
	
	for (int i = 0; i < objQ.size(); i++) {

		const tre::Object& currObj = objQ[i];

		//Set vertex buffer
		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;
		_context->IASetVertexBuffers(0, 1, currObj.pObjMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		_context->IASetIndexBuffer(currObj.pObjMesh->pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//set shader resc view and sampler
		_context->PSSetShaderResources(0, 1, currObj.pObjTexture->pShaderResView.GetAddressOf());

		//Config and set const buffer
		tre::ConstantBuffer::setObjConstBuffer(
			_device, _context,
			tre::Maths::createTransformationMatrix(currObj.objScale, currObj.objRotation, currObj.objPos),
			currObj.objColor,
			currObj.isObjWithTexture,
			currObj.isObjWithNormalMap
		);

		// set normal map
		if (currObj.isObjWithNormalMap) {
			_context->PSSetShaderResources(1, 1, currObj.pObjNormalMap->pShaderResView.GetAddressOf());
		}

		_context->DrawIndexed(currObj.pObjMesh->indexSize, 0, 0);
	}
}

void Renderer::debugDraw(ID3D11RasterizerState* rasterizerState, std::vector<Object>& objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderMode) {

	configureContext(renderMode);
	_context->RSSetState(rasterizerState);

	for (int i = 0; i < objQ.size(); i++) {

		tre::Object& currObj = objQ[i];

		//Set vertex buffer
		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;
		_context->IASetVertexBuffers(0, 1, mesh.pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		_context->IASetIndexBuffer(mesh.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//set shader resc view and sampler
		_context->PSSetShaderResources(0, 1, currObj.pObjTexture->pShaderResView.GetAddressOf());

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
			_device, _context,
			transformM,
			currObj.objColor,
			currObj.isObjWithTexture,
			currObj.isObjWithNormalMap
		);

		// set normal map
		if (currObj.isObjWithNormalMap) {
			_context->PSSetShaderResources(1, 1, currObj.pObjNormalMap->pShaderResView.GetAddressOf());
		}

		_context->DrawIndexed(mesh.indexSize, 0, 0);
	}
}

}