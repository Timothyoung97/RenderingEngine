#include "rendererSSAO.h"

#include "utility.h"
#include "engine.h"

extern tre::Engine* pEngine;

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

SSAOKernalStruct RendererSSAO::createSSAOKernalStruct(const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius) {
	SSAOKernalStruct ssaoKernalStruct;
	std::copy(kernalSamples.begin(), kernalSamples.end(), ssaoKernalStruct.kernalSamples);
	ssaoKernalStruct.sampleRadius = sampleRadius;

	return ssaoKernalStruct;
}

void RendererSSAO::fullscreenPass(Graphics& graphics, const Scene& scene, const Camera& cam) {
	if (!graphics.setting.ssaoSwitch) return;

	const char* name = ToString(RENDER_MODE::SSAO_FULLSCREEN_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	//PROFILE_GPU_SCOPED("Fullscreen Pass");

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Set const buffer for SSAO Kernal
	ID3D11Buffer* constBufferSSAOKernal = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::SSAOKernalStruct));
	{
		tre::SSAOKernalStruct ssaoKernalStruct = createSSAOKernalStruct(graphics._ssao.ssaoKernalSamples, graphics.setting.ssaoSampleRadius);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferSSAOKernal, &ssaoKernalStruct, (UINT)sizeof(tre::SSAOKernalStruct));
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
		contextD.Get()->PSSetConstantBuffers(3u, 1u, &constBufferSSAOKernal);
		contextD.Get()->PSSetShaderResources(1, 1, graphics._gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		contextD.Get()->PSSetShaderResources(4, 1, graphics._depthbuffer.pDepthStencilShaderRescView.GetAddressOf()); //depth
		contextD.Get()->PSSetSamplers(0, 1, graphics._sampler.pSamplerStateMinMagMipLinearWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(1, 1, graphics._sampler.pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf());
		contextD.Get()->PSSetSamplers(2, 1, graphics._sampler.pSamplerStateMinMagMipPtWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(3, 1, graphics._sampler.pSamplerStateMinMagMipPtClamp.GetAddressOf());

		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		contextD.Get()->OMSetRenderTargets(1, graphics._ssao.ssaoResultTexture2dRTV.GetAddressOf(), nullptr);
	}

	contextD.Get()->Draw(6, 0);

	// clean up
	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
		graphics.bufferQueue.push_back(constBufferSSAOKernal);
	}
}

void RendererSSAO::fullscreenBlurPass(const Graphics& graphics) {
	if (!graphics.setting.ssaoSwitch) return;

	const char* name = ToString(RENDER_MODE::SSAO_BLURRING_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	//PROFILE_GPU_SCOPED("Fullscreen Pass");

	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(nullptr);
		contextD.Get()->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		contextD.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		contextD.Get()->PSSetShader(_textureBlurPixelShader.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetShaderResources(6, 1, graphics._ssao.ssaoResultTexture2dSRV.GetAddressOf()); // normal
		contextD.Get()->PSSetSamplers(0, 1, graphics._sampler.pSamplerStateMinMagMipLinearWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(1, 1, graphics._sampler.pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf());
		contextD.Get()->PSSetSamplers(2, 1, graphics._sampler.pSamplerStateMinMagMipPtWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(3, 1, graphics._sampler.pSamplerStateMinMagMipPtClamp.GetAddressOf());

		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		contextD.Get()->OMSetRenderTargets(1, graphics._ssao.ssaoBlurredTexture2dRTV.GetAddressOf(), nullptr);
	}

	contextD.Get()->Draw(6, 0);
}

void RendererSSAO::render(Graphics& graphics, const Scene& scene, const Camera& cam) {
	MICROPROFILE_SCOPE_CSTR("CPU SSAO PASS");
	//PROFILE_GPU_SCOPED("GPU SSAO Pass");
	fullscreenPass(graphics, scene, cam);
	fullscreenBlurPass(graphics);

	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}
}

}