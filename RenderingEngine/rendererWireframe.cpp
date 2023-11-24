#include "rendererWireframe.h"

#include <set>

#include "utility.h"

namespace tre {

// Constructor: Create and allocate 
RendererWireframe::RendererWireframe(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	
	wireframeCube = CubeMesh(_device);
	wireframeSphere = SphereMesh(_device, 10, 10);
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", _device);
	_debugPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_debug.bin", _device);

	_vertexShaderInstanced.create(basePathWstr + L"shaders\\bin\\vertex_shader_instancedRendering.bin", _device);
	_debugPixelShaderInstanced.create(basePathWstr + L"shaders\\bin\\pixel_shader_instanced_wireframe.bin", _device);
}

Mesh* RendererWireframe::selectWireframeMesh(BoundVolumeEnum typeOfBound) {
	switch (typeOfBound) {
	case AABBBoundingBox:
		return &wireframeCube;
		break;
	case RitterBoundingSphere: // fall through
	case NaiveBoundingSphere:
	default:
		return &wireframeSphere;
		break;
	}
}

void RendererWireframe::setConstBufferCamViewProj(Graphics& graphic, const Camera& cam) {
	if (!graphic.setting.showBoundingVolume) return;
	ID3D11Buffer* constBufferCamViewProj = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::ViewProjectionStruct));
	{
		// Update const buffer and binding
		tre::ViewProjectionStruct vpStruct = tre::CommonStructUtility::createViewProjectionStruct(cam.camViewProjection);
		tre::Buffer::updateConstBufferData(_context, constBufferCamViewProj, &vpStruct, (UINT)sizeof(tre::ViewProjectionStruct));

		_context->VSSetConstantBuffers(0u, 1u, &constBufferCamViewProj);
	}
	graphic.bufferQueue.push_back(constBufferCamViewProj);
}

void RendererWireframe::draw(Graphics& graphics, const std::vector<Object*>& objQ) {
	if (objQ.size() == 0 || !graphics.setting.showBoundingVolume) return;

	const char* name = ToString(RENDER_MODE::WIREFRAME_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Draw Wireframe");

	// Select mesh to render based on bounding methods
	Mesh* meshToRender = selectWireframeMesh(graphics.setting.typeOfBound);

	// Configure Context
	{
		// Input Assembler
		UINT vertexStride = sizeof(Vertex), offset = 0u;
		_context->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		_context->IASetVertexBuffers(0, 1, meshToRender->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
		_context->IASetIndexBuffer(meshToRender->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateWireFrame.Get());

		_context->PSSetShader(_debugPixelShader.pShader.Get(), NULL, 0u);

		_context->OMSetBlendState(graphics._blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithoutDepthT.Get(), 0);
		_context->OMSetRenderTargets(1, &graphics.currRenderTargetView, nullptr);
	}

	// Create empty const buffer and pre bind the constant buffer
	ID3D11Buffer* constBufferModelInfo = tre::Buffer::createConstBuffer(_device, sizeof(tre::ModelInfoStruct));
	{
		_context->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
		_context->PSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
	}

	std::set<Object*> processed;
	for (int i = 0; i < objQ.size(); i++) {

		tre::Object* currObj = objQ[i];
		if (processed.contains(currObj)) {
			continue;
		}

		processed.insert(currObj);
		for (int j = 0; j < currObj->pObjMeshes.size(); j++) {

			if (!currObj->isInView[j]) {
				continue;
			}

			// Submit each object's data to const buffer
			{
				tre::ModelInfoStruct modelInfoStruct = tre::CommonStructUtility::createModelInfoStruct(currObj->_boundingVolumeTransformation[j], currObj->_boundingVolumeColor[j], 0u, 0u);
				tre::Buffer::updateConstBufferData(_context, constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
			}

			_context->DrawIndexed(meshToRender->indexSize, 0u, 0u);
		}
	}

	// push into buffer for cleaning in the next frame
	{
		graphics.bufferQueue.push_back(constBufferModelInfo);
	}
}

void RendererWireframe::drawInstanced(Graphics& graphics, const std::vector<Object*>& objQ) {
	if (objQ.size() == 0 || !graphics.setting.showBoundingVolume) return;
	const char* name = ToString(tre::RENDER_MODE::WIREFRAME_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Instanced Draw Wireframe");

	// Select mesh to render based on bounding methods
	Mesh* meshToRender = selectWireframeMesh(graphics.setting.typeOfBound);

	// Update instance buffer with wireframe mesh
	{
		graphics._instanceBuffer.updateBuffer(objQ, meshToRender);
	}

	{
		UINT vertexStride = sizeof(Vertex), offset = 0;
		_context->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		_context->IASetVertexBuffers(0, 1, meshToRender->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
		_context->IASetIndexBuffer(meshToRender->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		_context->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		_context->VSSetShaderResources(0u, 1, graphics._instanceBuffer.pInstanceBufferSRV.GetAddressOf()); // to do

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateWireFrame.Get());

		_context->PSSetShader(_debugPixelShaderInstanced.pShader.Get(), NULL, 0u);

		_context->OMSetBlendState(graphics._blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithoutDepthT.Get(), 0);
		_context->OMSetRenderTargets(1, &graphics.currRenderTargetView, nullptr);
	}

	// Create an empty const buffer 
	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::BatchInfoStruct));

	InstanceBatchInfo currBatchInfo = graphics._instanceBuffer.instanceBatchQueue[0];

	// update constant buffer for instanced draw call
	{
		tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
		tre::Buffer::updateConstBufferData(_context, constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));

		_context->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);
	}

	// Draw call
	_context->DrawIndexedInstanced(currBatchInfo.pBatchMesh->indexSize, currBatchInfo.quantity, 0u, 0u, 0u);

	// push into buffer for cleaning in the next frame
	{
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}
}

void RendererWireframe::render(Graphics& graphics, const Camera& cam, const Scene& scene) {
	PROFILE_GPU_SCOPED("Render Bounding Volume Wireframe");
	setConstBufferCamViewProj(graphics, cam);

	drawInstanced(graphics, scene._wireframeObjQ);		// for point lights
	drawInstanced(graphics, scene._pObjQ);				// for all opaque + transparent objects
}

}