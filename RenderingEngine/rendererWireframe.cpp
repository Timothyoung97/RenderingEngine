#include "rendererWireframe.h"

#include <set>

#include "utility.h"
#include "engine.h"
#include "dxdebug.h"

extern tre::Engine* pEngine;

namespace tre {

RendererWireframe::RendererWireframe() {
	this->init();
}

void RendererWireframe::init() {
	
	wireframeCube = CubeMesh(pEngine->device->device.Get());
	wireframeSphere = SphereMesh(pEngine->device->device.Get(), 10, 10);
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", pEngine->device->device.Get());
	_debugPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_debug.bin", pEngine->device->device.Get());

	_vertexShaderInstanced.create(basePathWstr + L"shaders\\bin\\vertex_shader_instancedRendering.bin", pEngine->device->device.Get());
	_debugPixelShaderInstanced.create(basePathWstr + L"shaders\\bin\\pixel_shader_instanced_wireframe.bin", pEngine->device->device.Get());
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
	ID3D11Buffer* constBufferCamViewProj = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::ViewProjectionStruct));
	{
		// Update const buffer and binding
		tre::ViewProjectionStruct vpStruct = tre::CommonStructUtility::createViewProjectionStruct(cam.camViewProjection);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferCamViewProj, &vpStruct, (UINT)sizeof(tre::ViewProjectionStruct));

		contextD.Get()->VSSetConstantBuffers(0u, 1u, &constBufferCamViewProj);
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
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		contextD.Get()->IASetVertexBuffers(0, 1, meshToRender->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
		contextD.Get()->IASetIndexBuffer(meshToRender->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		contextD.Get()->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateWireFrame.Get());

		contextD.Get()->PSSetShader(_debugPixelShader.pShader.Get(), NULL, 0u);

		contextD.Get()->OMSetBlendState(graphics._blendstate.transparency.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithoutDepthT.Get(), 0);
		contextD.Get()->OMSetRenderTargets(1, graphics.currRenderTargetView.GetAddressOf(), nullptr);
	}

	// Create empty const buffer and pre bind the constant buffer
	ID3D11Buffer* constBufferModelInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), sizeof(tre::ModelInfoStruct));
	{
		contextD.Get()->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
		contextD.Get()->PSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
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
				tre::Buffer::updateConstBufferData(contextD.Get(), constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
			}

			contextD.Get()->DrawIndexed(meshToRender->indexSize, 0u, 0u);
		}
	}

	// push into buffer for cleaning in the next frame
	{
		graphics.bufferQueue.push_back(constBufferModelInfo);
	}
}

void RendererWireframe::drawInstanced(Graphics& graphics, const std::vector<Object*>& objQ, InstanceBuffer& targetInstanceBuffer) {
	if (objQ.size() == 0 || !graphics.setting.showBoundingVolume) return;
	const char* name = ToString(tre::RENDER_MODE::WIREFRAME_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Instanced Draw Wireframe");

	// Select mesh to render based on bounding methods
	Mesh* meshToRender = selectWireframeMesh(graphics.setting.typeOfBound);

	// Update instance buffer with wireframe mesh
	{
		targetInstanceBuffer.updateBuffer(objQ, meshToRender);
	}

	{
		UINT vertexStride = sizeof(Vertex), offset = 0;
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		contextD.Get()->IASetVertexBuffers(0, 1, meshToRender->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
		contextD.Get()->IASetIndexBuffer(meshToRender->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		contextD.Get()->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		contextD.Get()->VSSetShaderResources(0u, 1, targetInstanceBuffer.pInstanceBufferSRV.GetAddressOf()); // to do

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateWireFrame.Get());

		contextD.Get()->PSSetShader(_debugPixelShaderInstanced.pShader.Get(), NULL, 0u);

		contextD.Get()->OMSetBlendState(graphics._blendstate.transparency.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithoutDepthT.Get(), 0);
		contextD.Get()->OMSetRenderTargets(1, graphics.currRenderTargetView.GetAddressOf(), nullptr);
	}

	// Create an empty const buffer 
	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BatchInfoStruct));

	InstanceBatchInfo currBatchInfo = targetInstanceBuffer.instanceBatchQueue[0];

	// update constant buffer for instanced draw call
	{
		tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));

		contextD.Get()->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);
	}

	// Draw call
	contextD.Get()->DrawIndexedInstanced(currBatchInfo.pBatchMesh->indexSize, currBatchInfo.quantity, 0u, 0u, 0u);

	// push into buffer for cleaning in the next frame
	{
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}

}

void RendererWireframe::render(Graphics& graphics, const Camera& cam, const Scene& scene) {
	if (!graphics.setting.showBoundingVolume) return;

	PROFILE_GPU_SCOPED("Render Bounding Volume Wireframe");
	setConstBufferCamViewProj(graphics, cam);

	drawInstanced(graphics, scene._wireframeObjQ, graphics._instanceBufferPointlights);
	drawInstanced(graphics, scene._pObjQ, graphics._instanceBufferMainView);
	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}
}

}