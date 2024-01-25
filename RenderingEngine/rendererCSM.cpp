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
	contextD.Get()->RSSetViewports(1, &graphics._viewport.shadowViewport);
	contextD.Get()->RSSetScissorRects(1, &graphics._rasterizer.rectArr[idx]);
}

void RendererCSM::drawInstanced(Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>>& objQ, int csmIdx) {
	if (objQ.size() == 0) return;

	// Profiling
	const char* name = ToString(RENDER_MODE::INSTANCED_SHADOW_M);
	MICROPROFILE_SCOPE_CSTR(name);
	//PROFILE_GPU_SCOPED("CSM Instanced Draw");

	// Update structured buffer for instanced draw call
	int numOfInstances = graphics._instanceBufferCSM[csmIdx].updateBuffer(objQ, contextD.Get());

	ComPtr<ID3D11DepthStencilView> shadowDepthStencilView;
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateDepthStencilView(
			graphics._depthbuffer.pShadowMapTexture.Get(), &depthStencilViewDesc, shadowDepthStencilView.GetAddressOf()
		));
	}

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
			graphics._instanceBufferCSM[csmIdx].pInstanceBuffer.Get(), &instanceBufferSRVDesc, instanceInfoBufferSRV.GetAddressOf()
		));
	}

	// Configure context for CMS Draw call
	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		contextD.Get()->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		contextD.Get()->VSSetShaderResources(0u, 1, instanceInfoBufferSRV.GetAddressOf());

		// use setShadowBufferDrawSection to select draw section
		contextD.Get()->RSSetState(graphics._rasterizer.pShadowRasterizerState.Get());

		// unbind shadow buffer as a resource, so that we can write to it
		contextD.Get()->PSSetShader(nullptr, NULL, 0u);
		contextD.Get()->PSSetShaderResources(3, 1, graphics.nullSRV);

		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		contextD.Get()->OMSetRenderTargets(0, nullptr, shadowDepthStencilView.Get());
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	// Create an empty const buffer 
	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BatchInfoStruct));
	contextD.Get()->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);

	for (int i = 0; i < graphics._instanceBufferCSM[csmIdx].instanceBatchQueue.size(); i++) {
		InstanceBatchInfo currBatchInfo = graphics._instanceBufferCSM[csmIdx].instanceBatchQueue[i];

		// update constant buffer for each instanced draw call
		{
			tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
			tre::Buffer::updateConstBufferData(contextD.Get(), constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));
		}

		// Update mesh vertex information
		{
			contextD.Get()->IASetVertexBuffers(0, 1, currBatchInfo.pBatchMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
			contextD.Get()->IASetIndexBuffer(currBatchInfo.pBatchMesh->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		}

		// Update texture information
		contextD.Get()->PSSetShaderResources(0u, 1u, graphics.nullSRV);

		contextD.Get()->PSSetShaderResources(1u, 1u, graphics.nullSRV);

		// Draw call
		contextD.Get()->DrawIndexedInstanced(currBatchInfo.pBatchMesh->indexSize, currBatchInfo.quantity, 0u, 0u, 0u);
	}

	// Pushing used const buffer to queue for cleaning
	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}
}

void RendererCSM::render(Graphics& graphics, Scene& scene, const Camera& cam, MicroProfiler& profiler) {

	MICROPROFILE_SCOPE_CSTR("CSM Section");
	MICROPROFILE_CONDITIONAL(MicroProfileThreadLogGpu* pMicroProfileLog = profiler.graphicsGpuThreadLog[0]);
	MICROPROFILE_GPU_BEGIN(contextD.Get(), pMicroProfileLog);

	ID3D11Buffer* constBufferCSMViewProj = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::ViewProjectionStruct));
	{
		MICROPROFILE_SECTIONGPUI_L(pMicroProfileLog, "CSM Section", tre::Utility::getRandomInt(INT_MAX));
		MICROPROFILE_SCOPEGPU_TOKEN_L(pMicroProfileLog, profiler.graphicsTokenGpuFrameIndex[0]);

		for (int viewIdx = 0; viewIdx < 4; viewIdx++) {

			MICROPROFILE_SCOPE_CSTR("CSM: Draw");

			// Render based on pointlight's view projection matrix
			{	
				setCSMViewport(graphics, viewIdx);

				// Const Buffer 
				{
					// Create struct info and submit data to const buffer
					tre::ViewProjectionStruct csmViewProjStruct = tre::CommonStructUtility::createViewProjectionStruct(scene.viewProjs[scene.csmViewBeginIdx + viewIdx]);
					tre::Buffer::updateConstBufferData(contextD.Get(), constBufferCSMViewProj, &csmViewProjStruct, (UINT)sizeof(tre::ViewProjectionStruct));

					// Binding 
					contextD.Get()->VSSetConstantBuffers(0u, 1u, &constBufferCSMViewProj);
				}

				graphics.stats.shadowCascadeOpaqueObjs[viewIdx] = scene._culledOpaqueObjQ[scene.csmViewBeginIdx + viewIdx].size();
			}

			// draw shadow only for opaque objects
			{
				MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "CSM: Draw", tre::Utility::getRandomInt(INT_MAX));
				drawInstanced(graphics, scene._culledOpaqueObjQ[scene.csmViewBeginIdx + viewIdx], scene.csmViewBeginIdx + viewIdx); // instanced
			}
		}
	}

	// Pushing used const buffer to queue for cleaning
	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferCSMViewProj);
	}

	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}

	profiler.graphicsMicroProfile[0] = MICROPROFILE_GPU_END(pMicroProfileLog);
}

}