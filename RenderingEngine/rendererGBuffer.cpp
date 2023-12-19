#include "rendererGBuffer.h"

#include "utility.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre { 

RendererGBuffer::RendererGBuffer() {
	this->init();
}

void RendererGBuffer::init() {
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderInstanced.create(basePathWstr + L"shaders\\bin\\vertex_shader_instancedRendering.bin", pEngine->device->device.Get());
	_pixelShaderInstanced.create(basePathWstr + L"shaders\\bin\\pixel_shader_instanced_gbuffer.bin", pEngine->device->device.Get());
}

void RendererGBuffer::render(Graphics& graphics, Scene& scene, Camera& cam) {
	if (scene._culledOpaqueObjQ[scene.camViewIdx].empty()) return;

	const char* name = ToString(RENDER_MODE::INSTANCED_DEFERRED_OPAQUE_M);
	MICROPROFILE_SCOPE_CSTR(name);

	// View Projection Const Buffer
	ID3D11Buffer* constBufferCamViewProj = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::ViewProjectionStruct));
	{
		// Update const buffer and binding
		{
			tre::ViewProjectionStruct vpStruct = tre::CommonStructUtility::createViewProjectionStruct(cam.camViewProjection);
			tre::Buffer::updateConstBufferData(pEngine->device->contextI.Get(), constBufferCamViewProj, &vpStruct, (UINT)sizeof(tre::ViewProjectionStruct));
			pEngine->device->contextI.Get()->VSSetConstantBuffers(0u, 1u, &constBufferCamViewProj);
		}
	}

	// Batching
	graphics._instanceBufferMainView.updateBuffer(scene._culledOpaqueObjQ[scene.camViewIdx]);

	// Create an empty const buffer 
	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BatchInfoStruct));

	{
		pEngine->device->contextI.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		pEngine->device->contextI.Get()->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		pEngine->device->contextI.Get()->VSSetShaderResources(0u, 1, graphics._instanceBufferMainView.pInstanceBufferSRV.GetAddressOf());
		pEngine->device->contextI.Get()->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);

		pEngine->device->contextI.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		pEngine->device->contextI.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		// unbind depth buffer as a shader resource, so that we can write to it
		pEngine->device->contextI.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		pEngine->device->contextI.Get()->PSSetShader(_pixelShaderInstanced.pShader.Get(), NULL, 0u);
		pEngine->device->contextI.Get()->PSSetShaderResources(4, 1, graphics.nullSRV);

		pEngine->device->contextI.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		pEngine->device->contextI.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		pEngine->device->contextI.Get()->OMSetRenderTargets(2, graphics._gBuffer.rtvs, graphics._depthbuffer.pDepthStencilView.Get());
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	PROFILE_GPU_SCOPED("G-Buffer Instanced Draw");
	for (int i = 0; i < graphics._instanceBufferMainView.instanceBatchQueue.size(); i++) {
		InstanceBatchInfo currBatchInfo = graphics._instanceBufferMainView.instanceBatchQueue[i];

		// update constant buffer for each instanced draw call
		{
			tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
			tre::Buffer::updateConstBufferData(pEngine->device->contextI.Get(), constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));
		}

		// Update mesh vertex information
		pEngine->device->contextI.Get()->IASetVertexBuffers(0, 1, currBatchInfo.pBatchMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
		pEngine->device->contextI.Get()->IASetIndexBuffer(currBatchInfo.pBatchMesh->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Update texture information
		pEngine->device->contextI.Get()->PSSetShaderResources(0u, 1u, graphics.nullSRV);
		if (currBatchInfo.isWithTexture) {
			pEngine->device->contextI.Get()->PSSetShaderResources(0u, 1u, currBatchInfo.pBatchTexture->pShaderResView.GetAddressOf());
		}

		pEngine->device->contextI.Get()->PSSetShaderResources(1u, 1u, graphics.nullSRV);
		if (currBatchInfo.hasNormMap) {
			pEngine->device->contextI.Get()->PSSetShaderResources(1u, 1u, currBatchInfo.pBatchNormalMap->pShaderResView.GetAddressOf());
		}

		// Draw call
		pEngine->device->contextI.Get()->DrawIndexedInstanced(currBatchInfo.pBatchMesh->indexSize, currBatchInfo.quantity, 0u, 0u, 0u);
	}

	// clean up
	{
		graphics.bufferQueue.push_back(constBufferCamViewProj);
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}
}
}