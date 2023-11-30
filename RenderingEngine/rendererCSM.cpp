#include "rendererCSM.h"

#include "microprofile.h"
#include "window.h"
#include "utility.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

RendererCSM::RendererCSM() {
	this->init();
}

void RendererCSM::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderInstanced.create(basePathWstr + L"shaders\\bin\\vertex_shader_instancedRendering.bin", pEngine->device->device.Get());
}

void RendererCSM::setCSMViewport(Graphics& graphics, int idx) {
	graphics._viewport.shadowViewport.TopLeftX = graphics._rasterizer.rectArr[idx].left;
	graphics._viewport.shadowViewport.TopLeftY = graphics._rasterizer.rectArr[idx].top;
	pEngine->device->context.Get()->RSSetViewports(1, &graphics._viewport.shadowViewport);
	pEngine->device->context.Get()->RSSetScissorRects(1, &graphics._rasterizer.rectArr[idx]);
}

void RendererCSM::drawInstanced(Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>>& objQ) {
	if (objQ.size() == 0) return;

	// Profiling
	const char* name = ToString(RENDER_MODE::INSTANCED_SHADOW_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("CSM Instanced Draw");

	// Update structured buffer for instanced draw call
	graphics._instanceBuffer.updateBuffer(objQ);

	// Configure context for CMS Drawc call
	{
		pEngine->device->context.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		pEngine->device->context.Get()->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		pEngine->device->context.Get()->VSSetShaderResources(0u, 1, graphics._instanceBuffer.pInstanceBufferSRV.GetAddressOf());

		// use setShadowBufferDrawSection to select draw section
		pEngine->device->context.Get()->RSSetState(graphics._rasterizer.pShadowRasterizerState.Get());

		// unbind shadow buffer as a resource, so that we can write to it
		pEngine->device->context.Get()->PSSetShader(nullptr, NULL, 0u);
		pEngine->device->context.Get()->PSSetShaderResources(3, 1, graphics.nullSRV);

		pEngine->device->context.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		pEngine->device->context.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		pEngine->device->context.Get()->OMSetRenderTargets(0, nullptr, graphics._depthbuffer.pShadowDepthStencilView.Get());
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	// Create an empty const buffer 
	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BatchInfoStruct));
	pEngine->device->context.Get()->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);

	for (int i = 0; i < graphics._instanceBuffer.instanceBatchQueue.size(); i++) {
		InstanceBatchInfo currBatchInfo = graphics._instanceBuffer.instanceBatchQueue[i];

		// update constant buffer for each instanced draw call
		{
			tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
			tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));
		}

		// Update mesh vertex information
		{
			pEngine->device->context.Get()->IASetVertexBuffers(0, 1, currBatchInfo.pBatchMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
			pEngine->device->context.Get()->IASetIndexBuffer(currBatchInfo.pBatchMesh->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		}

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

	// Pushing used const buffer to queue for cleaning
	{
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}
}

void RendererCSM::render(Graphics& graphics, Scene& scene, const Camera& cam) {

	ID3D11Buffer* constBufferCSMViewProj = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::ViewProjectionStruct));
	{
		PROFILE_GPU_SCOPED("CSM Quad Draw");

		for (int viewIdx = 0; viewIdx < 4; viewIdx++) {

			{	// Render based on pointlight's view projection matrix
				MICROPROFILE_SCOPE_CSTR("CSM Quad Obj Draw");

				setCSMViewport(graphics, viewIdx);

				// Const Buffer 
				{
					// Create struct info and submit data to const buffer
					tre::ViewProjectionStruct csmViewProjStruct = tre::CommonStructUtility::createViewProjectionStruct(scene.viewProjs[scene.csmViewBeginIdx + viewIdx]);
					tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), constBufferCSMViewProj, &csmViewProjStruct, (UINT)sizeof(tre::ViewProjectionStruct));

					// Binding 
					pEngine->device->context.Get()->VSSetConstantBuffers(0u, 1u, &constBufferCSMViewProj);
				}

				graphics.stats.shadowCascadeOpaqueObjs[scene.csmViewBeginIdx + viewIdx] = scene._culledOpaqueObjQ.size();
			}

			// draw shadow only for opaque objects
			//renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::SHADOW_M); // non instanced
			drawInstanced(graphics, scene._culledOpaqueObjQ[scene.csmViewBeginIdx + viewIdx]); // instanced
		}
	}

	// Pushing used const buffer to queue for cleaning
	{
		graphics.bufferQueue.push_back(constBufferCSMViewProj);
	}
}

}