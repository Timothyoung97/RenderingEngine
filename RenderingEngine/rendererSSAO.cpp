#include "rendererSSAO.h"

#include "utility.h"
#include "engine.h"

extern std::shared_ptr<tre::Engine> pEngine;

namespace tre {

RendererSSAO::RendererSSAO() {
	this->init();
}

void RendererSSAO::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", pEngine->device->device.Get());
	_ssaoPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_ssao_rendering.bin", pEngine->device->device.Get());
	_textureBlurPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_texture_blur.bin", pEngine->device->device.Get());
}

SSAOStruct RendererSSAO::createSSAOKernalStruct(float sampleRadius) {
	SSAOStruct ssaoKernalStruct;
	ssaoKernalStruct.sampleRadius = sampleRadius;
	ssaoKernalStruct.pad = XMFLOAT3(.0f, .0f, .0f);

	return ssaoKernalStruct;
}

void RendererSSAO::fullscreenPass(Graphics& graphics, const Scene& scene, const Camera& cam) {
	const char* name = ToString(RENDER_MODE::SSAO_FULLSCREEN_PASS);
	MICROPROFILE_SCOPE_CSTR(name);

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Set const buffer for SSAO Kernal
	ID3D11Buffer* constBufferSSAO = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::SSAOStruct));
	{
		tre::SSAOStruct ssaoKernalStruct = createSSAOKernalStruct(graphics.setting.ssaoSampleRadius);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferSSAO, &ssaoKernalStruct, (UINT)sizeof(tre::SSAOStruct));
	}

	// View Creations
	ComPtr<ID3D11RenderTargetView> ssaoResultTexture2dRTV;
	{
		D3D11_RENDER_TARGET_VIEW_DESC ssaoResultTexture2dRTVDesc;
		ZeroMemory(&ssaoResultTexture2dRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		ssaoResultTexture2dRTVDesc.Format = DXGI_FORMAT_R8_UNORM;
		ssaoResultTexture2dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		ssaoResultTexture2dRTVDesc.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			graphics._ssao.ssaoResultTexture2d.Get(), &ssaoResultTexture2dRTVDesc, ssaoResultTexture2dRTV.GetAddressOf()
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
		contextD.Get()->PSSetShader(_ssaoPixelShader.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		contextD.Get()->PSSetConstantBuffers(3u, 1u, &constBufferSSAO);
		contextD.Get()->PSSetShaderResources(1, 1, deferredNormalSRV.GetAddressOf()); // normal
		contextD.Get()->PSSetShaderResources(4, 1, depthStencilShaderRescView.GetAddressOf()); //depth
		contextD.Get()->PSSetSamplers(0, 1, graphics._sampler.pSamplerStateMinMagMipLinearWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(1, 1, graphics._sampler.pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf());
		contextD.Get()->PSSetSamplers(2, 1, graphics._sampler.pSamplerStateMinMagMipPtWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(3, 1, graphics._sampler.pSamplerStateMinMagMipPtClamp.GetAddressOf());

		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		contextD.Get()->OMSetRenderTargets(1, ssaoResultTexture2dRTV.GetAddressOf(), nullptr);
	}

	contextD.Get()->Draw(6, 0);

	// clean up
	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
		graphics.bufferQueue.push_back(constBufferSSAO);
	}
}

void RendererSSAO::fullscreenBlurPass(const Graphics& graphics) {
	const char* name = ToString(RENDER_MODE::SSAO_BLURRING_PASS);
	MICROPROFILE_SCOPE_CSTR(name);

	ComPtr<ID3D11ShaderResourceView> ssaoResultTexture2dSRV;
	ComPtr<ID3D11RenderTargetView> ssaoBlurredTexture2dRTV;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC ssaoResultTexture2dSRVDesc;
		ssaoResultTexture2dSRVDesc.Format = DXGI_FORMAT_R8_UNORM;
		ssaoResultTexture2dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		ssaoResultTexture2dSRVDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._ssao.ssaoResultTexture2d.Get(), &ssaoResultTexture2dSRVDesc, ssaoResultTexture2dSRV.GetAddressOf()
		));

		D3D11_RENDER_TARGET_VIEW_DESC ssaoResultTexture2dRTVDesc;
		ZeroMemory(&ssaoResultTexture2dRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		ssaoResultTexture2dRTVDesc.Format = DXGI_FORMAT_R8_UNORM;
		ssaoResultTexture2dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		ssaoResultTexture2dRTVDesc.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			graphics._ssao.ssaoBlurredTexture2d.Get(), &ssaoResultTexture2dRTVDesc, ssaoBlurredTexture2dRTV.GetAddressOf()
		));
	}

	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(nullptr);
		contextD.Get()->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		contextD.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		contextD.Get()->PSSetShader(_textureBlurPixelShader.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetShaderResources(6, 1, ssaoResultTexture2dSRV.GetAddressOf()); // normal
		contextD.Get()->PSSetSamplers(0, 1, graphics._sampler.pSamplerStateMinMagMipLinearWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(1, 1, graphics._sampler.pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf());
		contextD.Get()->PSSetSamplers(2, 1, graphics._sampler.pSamplerStateMinMagMipPtWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(3, 1, graphics._sampler.pSamplerStateMinMagMipPtClamp.GetAddressOf());

		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		contextD.Get()->OMSetRenderTargets(1, ssaoBlurredTexture2dRTV.GetAddressOf(), nullptr);
	}

	contextD.Get()->Draw(6, 0);
}

void RendererSSAO::render(Graphics& graphics, const Scene& scene, const Camera& cam, MicroProfiler& profiler) {
	if (!graphics.setting.ssaoSwitch) return;

	MICROPROFILE_SCOPE_CSTR("SSAO Section");
	profiler.graphicsGpuThreadLogStatus[2] = 1;
	MICROPROFILE_CONDITIONAL(MicroProfileThreadLogGpu* pMicroProfileLog = profiler.graphicsGpuThreadLog[2]);
	MICROPROFILE_GPU_BEGIN(contextD.Get(), pMicroProfileLog);

	{
		MICROPROFILE_SECTIONGPUI_L(pMicroProfileLog, "SSAO Section", tre::Utility::getRandomInt(INT_MAX));
		MICROPROFILE_SCOPEGPU_TOKEN_L(pMicroProfileLog, profiler.graphicsTokenGpuFrameIndex[2]);
		{
			MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "SSAO: Occulsion Draw", tre::Utility::getRandomInt(INT_MAX));
			fullscreenPass(graphics, scene, cam);
		}
		{
			MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "SSAO: Occulsion Blur", tre::Utility::getRandomInt(INT_MAX));
			fullscreenBlurPass(graphics);
		}
	}

	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}

	profiler.graphicsMicroProfile[2] = MICROPROFILE_GPU_END(pMicroProfileLog);
}

}