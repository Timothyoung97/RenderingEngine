#include "rendererWireframe.h"

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

	{
		std::lock_guard<std::mutex> lock(graphic.bufferQueueMutex);
		graphic.bufferQueue.push_back(constBufferCamViewProj);
	}
}

void RendererWireframe::draw(Graphics& graphics, const std::vector<Object*>& objQ) {
	if (objQ.size() == 0 || !graphics.setting.showBoundingVolume) return;

	const char* name = ToString(RENDER_MODE::WIREFRAME_M);
	MICROPROFILE_SCOPE_CSTR(name);

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
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferModelInfo);
	}
}

void RendererWireframe::drawInstanced(Graphics& graphics, const std::vector<Object*>& objQ, InstanceBuffer& targetInstanceBuffer) {
	if (objQ.size() == 0 || !graphics.setting.showBoundingVolume) return;
	MICROPROFILE_SCOPE_CSTR("Wireframe: Draw Call");

	// Select mesh to render based on bounding methods
	Mesh* meshToRender = selectWireframeMesh(graphics.setting.typeOfBound);

	// Update instance buffer with wireframe mesh
	int numOfInstances = targetInstanceBuffer.updateBuffer(objQ, meshToRender, contextD.Get());

	ComPtr<ID3D11ShaderResourceView> instanceInfoBufferSRV;
	{
		D3D11_BUFFER_SRV instanceBufferSRV;
		ZeroMemory(&instanceBufferSRV, sizeof(D3D11_BUFFER_SRV));
		instanceBufferSRV.FirstElement = 0u;
		instanceBufferSRV.NumElements = numOfInstances;

		D3D11_SHADER_RESOURCE_VIEW_DESC instanceBufferSRVDesc;
		instanceBufferSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		instanceBufferSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		instanceBufferSRVDesc.Buffer = instanceBufferSRV;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			targetInstanceBuffer.pInstanceBuffer.Get(), &instanceBufferSRVDesc, instanceInfoBufferSRV.GetAddressOf()
		));
	}

	{
		UINT vertexStride = sizeof(Vertex), offset = 0;
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		contextD.Get()->IASetVertexBuffers(0, 1, meshToRender->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
		contextD.Get()->IASetIndexBuffer(meshToRender->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		contextD.Get()->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		contextD.Get()->VSSetShaderResources(0u, 1, instanceInfoBufferSRV.GetAddressOf()); // to do

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
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}

}

void RendererWireframe::render(Graphics& graphics, const Camera& cam, const Scene& scene, MicroProfiler& profiler) {
	if (!graphics.setting.showBoundingVolume) return;

	MICROPROFILE_SCOPE_CSTR("Wireframe Section");

	profiler.graphicsGpuThreadLogStatus[7] = 1;
	MICROPROFILE_CONDITIONAL(MicroProfileThreadLogGpu * pMicroProfileLog = profiler.graphicsGpuThreadLog[7]);
	MICROPROFILE_GPU_BEGIN(contextD.Get(), pMicroProfileLog);
	{
		MICROPROFILE_SECTIONGPUI_L(pMicroProfileLog, "Wireframe Section", tre::Utility::getRandomInt(INT_MAX));
		MICROPROFILE_SCOPEGPU_TOKEN_L(pMicroProfileLog, profiler.graphicsTokenGpuFrameIndex[7]);

		setConstBufferCamViewProj(graphics, cam);

		{	
			MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "Wireframe: Point lights", tre::Utility::getRandomInt(INT_MAX));
			drawInstanced(graphics, scene._wireframeObjQ, graphics._instanceBufferPointlights);
		}
		
		{
			MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "Wireframe: All scene objects", tre::Utility::getRandomInt(INT_MAX));
			drawInstanced(graphics, scene._pObjQ, graphics._instanceBufferWireframes);
		}
	}
	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}
	profiler.graphicsMicroProfile[7] = MICROPROFILE_GPU_END(pMicroProfileLog);
}

}