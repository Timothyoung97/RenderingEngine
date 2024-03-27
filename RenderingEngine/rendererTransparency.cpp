#include "rendererTransparency.h"

namespace tre {

RendererTransparency::RendererTransparency() {
	this->init();
}

void RendererTransparency::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", pEngine->device->device.Get());
	_forwardShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_forward.bin", pEngine->device->device.Get());
}

void RendererTransparency::render(Graphics& graphics, const Scene& scene, const Camera& cam, MicroProfiler& profiler) {
	if (scene._culledTransparentObjQ.empty()) return;

	MICROPROFILE_SCOPE_CSTR("Transparency Section");
	profiler.graphicsGpuThreadLogStatus[4] = 1;
	MICROPROFILE_CONDITIONAL(MicroProfileThreadLogGpu * pMicroProfileLog = profiler.graphicsGpuThreadLog[4]);
	MICROPROFILE_GPU_BEGIN(contextD.Get(), pMicroProfileLog);

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Create empty const buffer and pre bind the constant buffer
	ID3D11Buffer* constBufferModelInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), sizeof(tre::ModelInfoStruct));

	// Create Views
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

	ComPtr<ID3D11ShaderResourceView> shadowShaderRescView;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._depthbuffer.pShadowMapTexture.Get(), &shaderResourceViewDesc, shadowShaderRescView.GetAddressOf()
		));
	}

	ComPtr<ID3D11RenderTargetView> hdrRTV;
	{
		D3D11_RENDER_TARGET_VIEW_DESC hdrRTVDesc;
		ZeroMemory(&hdrRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		hdrRTVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		hdrRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hdrRTVDesc.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			graphics._hdrBuffer.pHdrBufferTexture.Get(), &hdrRTVDesc, hdrRTV.GetAddressOf()
		));
	}

	// Context Configuration
	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		contextD.Get()->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);
		contextD.Get()->VSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		contextD.Get()->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		contextD.Get()->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		contextD.Get()->PSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
		contextD.Get()->PSSetSamplers(0, 1, graphics._sampler.pSamplerStateMinMagMipLinearWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(1, 1, graphics._sampler.pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf());
		contextD.Get()->PSSetShaderResources(3, 1, shadowShaderRescView.GetAddressOf()); // shadow
		contextD.Get()->PSSetShaderResources(4, 1, graphics.nullSRV);

		contextD.Get()->OMSetBlendState(graphics._blendstate.transparency.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0);
		contextD.Get()->OMSetRenderTargets(1, hdrRTV.GetAddressOf(), depthStencilView.Get());
	}

	{
		MICROPROFILE_SECTIONGPUI_L(pMicroProfileLog, "Transparency Section", tre::Utility::getRandomInt(INT_MAX));
		MICROPROFILE_SCOPEGPU_TOKEN_L(pMicroProfileLog, profiler.graphicsTokenGpuFrameIndex[4]);
		MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "Transparency: Draw", tre::Utility::getRandomInt(INT_MAX));

		for (int i = 0; i < scene._culledTransparentObjQ.size(); i++) {

			UINT vertexStride = sizeof(Vertex);
			UINT offset = 0;

			//Set vertex buffer
			contextD.Get()->IASetVertexBuffers(0, 1, scene._culledTransparentObjQ[i].second->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

			//Set index buffer
			contextD.Get()->IASetIndexBuffer(scene._culledTransparentObjQ[i].second->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			//set shader resc view and sampler
			bool hasTexture = 0;
			bool hasNormal = 0;
			if (scene._culledTransparentObjQ[i].second->pMaterial->objTexture != nullptr) {
				// Create Shader Resource view
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
				shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, -1);

				ComPtr<ID3D11ShaderResourceView> pTextureSRV;
				CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
					scene._culledTransparentObjQ[i].second->pMaterial->objTexture->pTextureResource.Get(), &shaderResViewDesc, pTextureSRV.GetAddressOf()
				));

				contextD.Get()->PSSetShaderResources(0, 1, pTextureSRV.GetAddressOf());
				hasTexture = 1;
			}

			// set normal map
			if (scene._culledTransparentObjQ[i].second->pMaterial->objNormalMap != nullptr) {

				// Create Shader Resource view
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
				shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, -1);

				ComPtr<ID3D11ShaderResourceView> pNormalTextureSRV;
				CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
					scene._culledTransparentObjQ[i].second->pMaterial->objNormalMap->pTextureResource.Get(), &shaderResViewDesc, pNormalTextureSRV.GetAddressOf()
				));

				contextD.Get()->PSSetShaderResources(1, 1, pNormalTextureSRV.GetAddressOf());
				hasNormal = 1;
			}

			// Submit each object's data to const buffer
			{
				tre::ModelInfoStruct modelInfoStruct = tre::CommonStructUtility::createModelInfoStruct(scene._culledTransparentObjQ[i].first->_transformationFinal, scene._culledTransparentObjQ[i].second->pMaterial->baseColor, hasTexture, hasNormal);
				tre::Buffer::updateConstBufferData(contextD.Get(), constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
			}

			contextD.Get()->DrawIndexed(scene._culledTransparentObjQ[i].second->indexSize, 0, 0);
		}
	}

	// clean up
	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
		graphics.bufferQueue.push_back(constBufferModelInfo);
	}

	{	
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}

	profiler.graphicsMicroProfile[4] = MICROPROFILE_GPU_END(pMicroProfileLog);
}

}