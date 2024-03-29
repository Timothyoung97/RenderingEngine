#include "rendererEnvironmentLighting.h"

namespace tre {

RendererEnvironmentLighting::RendererEnvironmentLighting() {
	this->init();
}

void RendererEnvironmentLighting::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", pEngine->device->device.Get());
	_deferredShaderLightingEnv.create(basePathWstr + L"shaders\\bin\\pixel_shader_deferred_lighting_env.bin", pEngine->device->device.Get());
}

void RendererEnvironmentLighting::render(Graphics& graphics, const Scene& scene, const Camera& cam, MicroProfiler& profiler) {
	MICROPROFILE_SCOPE_CSTR("Environment Lighting Section");
	profiler.graphicsGpuThreadLogStatus[3] = 1;
	MICROPROFILE_CONDITIONAL(MicroProfileThreadLogGpu * pMicroProfileLog = profiler.graphicsGpuThreadLog[3]);
	MICROPROFILE_GPU_BEGIN(contextD.Get(), pMicroProfileLog);

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Create Views
	ComPtr<ID3D11ShaderResourceView> ssaoBlurredTexture2dSRV;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC ssaoResultTexture2dSRVDesc;
		ssaoResultTexture2dSRVDesc.Format = DXGI_FORMAT_R8_UNORM;
		ssaoResultTexture2dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		ssaoResultTexture2dSRVDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._ssao.ssaoBlurredTexture2d.Get(), &ssaoResultTexture2dSRVDesc, ssaoBlurredTexture2dSRV.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> depthStencilShaderRescView;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._depthbuffer.pDepthStencilTexture.Get(), &shaderResourceViewDesc, depthStencilShaderRescView.GetAddressOf()
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

	ComPtr<ID3D11ShaderResourceView> deferredAlbedoSRV;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
		shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._gBuffer.pGBufferTextureAlbedo.Get(), &shaderResViewDesc, deferredAlbedoSRV.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> deferredNormalSRV;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
		shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._gBuffer.pGBufferTextureNormal.Get(), &shaderResViewDesc, deferredNormalSRV.GetAddressOf()
		));
	}

	// Context Configuration
	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(nullptr);
		contextD.Get()->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		contextD.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		contextD.Get()->PSSetShader(_deferredShaderLightingEnv.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		contextD.Get()->PSSetShaderResources(0, 1, deferredAlbedoSRV.GetAddressOf()); // albedo
		contextD.Get()->PSSetShaderResources(1, 1, deferredNormalSRV.GetAddressOf()); // normal
		contextD.Get()->PSSetShaderResources(3, 1, shadowShaderRescView.GetAddressOf()); // shadow
		contextD.Get()->PSSetShaderResources(4, 1, depthStencilShaderRescView.GetAddressOf()); //depth
		contextD.Get()->PSSetShaderResources(7, 1, ssaoBlurredTexture2dSRV.GetAddressOf()); // ssao
		contextD.Get()->PSSetSamplers(0, 1, graphics._sampler.pSamplerStateMinMagMipLinearWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(1, 1, graphics._sampler.pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf());

		contextD.Get()->OMSetRenderTargets(1, hdrRTV.GetAddressOf(), nullptr);
		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
	}

	{
		MICROPROFILE_SECTIONGPUI_L(pMicroProfileLog, "Environment Lighting Section", tre::Utility::getRandomInt(INT_MAX));
		MICROPROFILE_SCOPEGPU_TOKEN_L(pMicroProfileLog, profiler.graphicsTokenGpuFrameIndex[3]);
		MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "Environment Lighting: Draw", tre::Utility::getRandomInt(INT_MAX));
		contextD.Get()->Draw(6, 0);
	}

	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
	}

	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}

	profiler.graphicsMicroProfile[3] = MICROPROFILE_GPU_END(pMicroProfileLog);
}
} 