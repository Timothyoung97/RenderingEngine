#include "rendererGBuffer.h"

namespace tre { 

RendererGBuffer::RendererGBuffer() {
	this->init();
}

void RendererGBuffer::init() {
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderInstanced.create(basePathWstr + L"shaders\\bin\\vertex_shader_instancedRendering.bin", pEngine->device->device.Get());
	_pixelShaderInstanced.create(basePathWstr + L"shaders\\bin\\pixel_shader_instanced_gbuffer.bin", pEngine->device->device.Get());
}

void RendererGBuffer::render(Graphics& graphics, Scene& scene, Camera& cam, MicroProfiler& profiler) {
	if (scene._culledOpaqueObjQ[scene.camViewIdx].empty()) return;

	MICROPROFILE_SCOPE_CSTR("G-Buffer Section");
	profiler.graphicsGpuThreadLogStatus[1] = 1;
	MICROPROFILE_CONDITIONAL(MicroProfileThreadLogGpu * pMicroProfileLog = profiler.graphicsGpuThreadLog[1]);
	MICROPROFILE_GPU_BEGIN(contextD.Get(), pMicroProfileLog);

	// View Projection Const Buffer
	ID3D11Buffer* constBufferCamViewProj = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::ViewProjectionStruct));
	{
		// Update const buffer and binding
		{
			tre::ViewProjectionStruct vpStruct = tre::CommonStructUtility::createViewProjectionStruct(cam.camViewProjection);
			tre::Buffer::updateConstBufferData(contextD.Get(), constBufferCamViewProj, &vpStruct, (UINT)sizeof(tre::ViewProjectionStruct));
			contextD.Get()->VSSetConstantBuffers(0u, 1u, &constBufferCamViewProj);
		}
	}

	// Batching
	int numOfInstances = graphics._instanceBufferMainView.updateBuffer(scene._culledOpaqueObjQ[scene.camViewIdx], contextD.Get());

	// Create an empty const buffer 
	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BatchInfoStruct));

	//Create View
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateDepthStencilView(
			graphics._depthbuffer.pDepthStencilTexture.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf()
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
			graphics._instanceBufferMainView.pInstanceBuffer.Get(), &instanceBufferSRVDesc, instanceInfoBufferSRV.GetAddressOf()
		));
	}

	ID3D11RenderTargetView* rtvs[2] = { nullptr, nullptr };
	ComPtr<ID3D11RenderTargetView> pRenderTargetViewDeferredAlbedo;
	ComPtr<ID3D11RenderTargetView> pRenderTargetViewDeferredNormal;
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvd;
		ZeroMemory(&rtvd, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			graphics._gBuffer.pGBufferTextureAlbedo.Get(), &rtvd, pRenderTargetViewDeferredAlbedo.GetAddressOf()
		));

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			graphics._gBuffer.pGBufferTextureNormal.Get(), &rtvd, pRenderTargetViewDeferredNormal.GetAddressOf()
		));

		rtvs[0] = pRenderTargetViewDeferredAlbedo.Get(), rtvs[1] = pRenderTargetViewDeferredNormal.Get();
	}

	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		contextD.Get()->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		contextD.Get()->VSSetShaderResources(0u, 1, instanceInfoBufferSRV.GetAddressOf());
		contextD.Get()->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		// unbind depth buffer as a shader resource, so that we can write to it
		contextD.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		contextD.Get()->PSSetShader(_pixelShaderInstanced.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetShaderResources(4, 1, graphics.nullSRV);
		contextD.Get()->PSSetSamplers(0, 1, graphics._sampler.pSamplerStateMinMagMipLinearWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(1, 1, graphics._sampler.pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf());

		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		contextD.Get()->OMSetRenderTargets(2, rtvs, depthStencilView.Get());
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	{
		MICROPROFILE_SECTIONGPUI_L(pMicroProfileLog, "G-Buffer Section", tre::Utility::getRandomInt(INT_MAX));
		MICROPROFILE_SCOPEGPU_TOKEN_L(pMicroProfileLog, profiler.graphicsTokenGpuFrameIndex[1]);
		MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "G-Buffer: Draw", tre::Utility::getRandomInt(INT_MAX));

		for (int i = 0; i < graphics._instanceBufferMainView.instanceBatchQueue.size(); i++) {
			InstanceBatchInfo currBatchInfo = graphics._instanceBufferMainView.instanceBatchQueue[i];

			// update constant buffer for each instanced draw call
			{
				tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
				tre::Buffer::updateConstBufferData(contextD.Get(), constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));
			}

			// Update mesh vertex information
			contextD.Get()->IASetVertexBuffers(0, 1, currBatchInfo.pBatchMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
			contextD.Get()->IASetIndexBuffer(currBatchInfo.pBatchMesh->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			// Update texture information
			contextD.Get()->PSSetShaderResources(0u, 1u, graphics.nullSRV);
			if (currBatchInfo.isWithTexture) {

				// Create Shader Resource view
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
				shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, -1);

				ComPtr<ID3D11ShaderResourceView> pTextureSRV;
				CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
					currBatchInfo.pBatchTexture->pTextureResource.Get(), &shaderResViewDesc, pTextureSRV.GetAddressOf()
				));

				contextD.Get()->PSSetShaderResources(0u, 1u, pTextureSRV.GetAddressOf());
			}

			contextD.Get()->PSSetShaderResources(1u, 1u, graphics.nullSRV);
			if (currBatchInfo.hasNormMap) {

				// Create Shader Resource view
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
				shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, -1);

				ComPtr<ID3D11ShaderResourceView> pNormalTextureSRV;
				CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
					currBatchInfo.pBatchNormalMap->pTextureResource.Get(), &shaderResViewDesc, pNormalTextureSRV.GetAddressOf()
				));

				contextD.Get()->PSSetShaderResources(1u, 1u, pNormalTextureSRV.GetAddressOf());
			}

			// Draw call
			contextD.Get()->DrawIndexedInstanced(currBatchInfo.pBatchMesh->indexSize, currBatchInfo.quantity, 0u, 0u, 0u);
		}
	}

	// clean up
	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferCamViewProj);
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}

	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}

	profiler.graphicsMicroProfile[1] = MICROPROFILE_GPU_END(pMicroProfileLog);

}
}