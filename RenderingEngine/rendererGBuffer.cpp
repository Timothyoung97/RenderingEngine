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
			tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), constBufferCamViewProj, &vpStruct, (UINT)sizeof(tre::ViewProjectionStruct));
			pEngine->device->context.Get()->VSSetConstantBuffers(0u, 1u, &constBufferCamViewProj);
		}
	}

	// Batching
	graphics._instanceBuffer.updateBuffer(scene._culledOpaqueObjQ[scene.camViewIdx]);

	// Create an empty const buffer 
	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BatchInfoStruct));

	{
		pEngine->device->context.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		pEngine->device->context.Get()->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		pEngine->device->context.Get()->VSSetShaderResources(0u, 1, graphics._instanceBuffer.pInstanceBufferSRV.GetAddressOf());
		pEngine->device->context.Get()->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);

		pEngine->device->context.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		pEngine->device->context.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		// unbind depth buffer as a shader resource, so that we can write to it
		pEngine->device->context.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		pEngine->device->context.Get()->PSSetShader(_pixelShaderInstanced.pShader.Get(), NULL, 0u);
		pEngine->device->context.Get()->PSSetShaderResources(4, 1, graphics.nullSRV);

		pEngine->device->context.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		pEngine->device->context.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		pEngine->device->context.Get()->OMSetRenderTargets(2, graphics._gBuffer.rtvs, graphics._depthbuffer.pDepthStencilView.Get());
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	PROFILE_GPU_SCOPED("G-Buffer Instanced Draw");
	for (int i = 0; i < graphics._instanceBuffer.instanceBatchQueue.size(); i++) {
		InstanceBatchInfo currBatchInfo = graphics._instanceBuffer.instanceBatchQueue[i];

		// update constant buffer for each instanced draw call
		{
			tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
			tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));
		}

		// Update mesh vertex information
		pEngine->device->context.Get()->IASetVertexBuffers(0, 1, currBatchInfo.pBatchMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
		pEngine->device->context.Get()->IASetIndexBuffer(currBatchInfo.pBatchMesh->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Update texture information
		pEngine->device->context.Get()->PSSetShaderResources(0u, 1u, graphics.nullSRV);
		if (currBatchInfo.isWithTexture) {
			pEngine->device->context.Get()->PSSetShaderResources(0u, 1u, currBatchInfo.pBatchTexture->pShaderResView.GetAddressOf());
		}

		pEngine->device->context.Get()->PSSetShaderResources(1u, 1u, graphics.nullSRV);
		if (currBatchInfo.hasNormMap) {
			pEngine->device->context.Get()->PSSetShaderResources(1u, 1u, currBatchInfo.pBatchNormalMap->pShaderResView.GetAddressOf());
		}

		// Draw call
		pEngine->device->context.Get()->DrawIndexedInstanced(currBatchInfo.pBatchMesh->indexSize, currBatchInfo.quantity, 0u, 0u, 0u);
	}

	// clean up
	{
		graphics.bufferQueue.push_back(constBufferCamViewProj);
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}
}
}