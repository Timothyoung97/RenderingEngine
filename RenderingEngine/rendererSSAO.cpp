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
	PROFILE_GPU_SCOPED("Fullscreen Pass");

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(pEngine->device->contextI.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Set const buffer for SSAO Kernal
	ID3D11Buffer* constBufferSSAOKernal = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::SSAOKernalStruct));
	{
		tre::SSAOKernalStruct ssaoKernalStruct = createSSAOKernalStruct(graphics._ssao.ssaoKernalSamples, graphics.setting.ssaoSampleRadius);
		tre::Buffer::updateConstBufferData(pEngine->device->contextI.Get(), constBufferSSAOKernal, &ssaoKernalStruct, (UINT)sizeof(tre::SSAOKernalStruct));
	}

	// Context Configuration
	{
		pEngine->device->contextI.Get()->IASetInputLayout(nullptr);
		pEngine->device->contextI.Get()->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		pEngine->device->contextI.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		pEngine->device->contextI.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		pEngine->device->contextI.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		pEngine->device->contextI.Get()->PSSetShader(_ssaoPixelShader.pShader.Get(), NULL, 0u);
		pEngine->device->contextI.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		pEngine->device->contextI.Get()->PSSetConstantBuffers(3u, 1u, &constBufferSSAOKernal);
		pEngine->device->contextI.Get()->PSSetShaderResources(1, 1, graphics._gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		pEngine->device->contextI.Get()->PSSetShaderResources(4, 1, graphics._depthbuffer.pDepthStencilShaderRescView.GetAddressOf()); //depth

		pEngine->device->contextI.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		pEngine->device->contextI.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		pEngine->device->contextI.Get()->OMSetRenderTargets(1, graphics._ssao.ssaoResultTexture2dRTV.GetAddressOf(), nullptr);
	}

	pEngine->device->contextI.Get()->Draw(6, 0);

	// clean up
	{
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
		graphics.bufferQueue.push_back(constBufferSSAOKernal);
	}
}

void RendererSSAO::fullscreenBlurPass(const Graphics& graphics) {
	if (!graphics.setting.ssaoSwitch) return;

	const char* name = ToString(RENDER_MODE::SSAO_BLURRING_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Fullscreen Pass");

	{
		pEngine->device->contextI.Get()->IASetInputLayout(nullptr);
		pEngine->device->contextI.Get()->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		pEngine->device->contextI.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		pEngine->device->contextI.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		pEngine->device->contextI.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		pEngine->device->contextI.Get()->PSSetShader(_textureBlurPixelShader.pShader.Get(), NULL, 0u);
		pEngine->device->contextI.Get()->PSSetShaderResources(6, 1, graphics._ssao.ssaoResultTexture2dSRV.GetAddressOf()); // normal

		pEngine->device->contextI.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		pEngine->device->contextI.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		pEngine->device->contextI.Get()->OMSetRenderTargets(1, graphics._ssao.ssaoBlurredTexture2dRTV.GetAddressOf(), nullptr);
	}

	pEngine->device->contextI.Get()->Draw(6, 0);
}

void RendererSSAO::render(Graphics& graphics, const Scene& scene, const Camera& cam) {
	MICROPROFILE_SCOPE_CSTR("CPU SSAO PASS");
	PROFILE_GPU_SCOPED("GPU SSAO Pass");
	fullscreenPass(graphics, scene, cam);
	fullscreenBlurPass(graphics);
}

}