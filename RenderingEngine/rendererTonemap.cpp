#include "rendererTonemap.h"

#include "utility.h"
#include "window.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

RendererTonemap::RendererTonemap() {
	this->init();
}

void RendererTonemap::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();

	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", pEngine->device->device.Get());
	_hdrPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_hdr_rendering.bin", pEngine->device->device.Get());
}

HDRStruct RendererTonemap::createHDRStruct(float middleGrey) {
	HDRStruct hdrStruct;
	hdrStruct.middleGrey = middleGrey;

	return hdrStruct;
}

void RendererTonemap::setConstBufferHDR(Graphics& graphics) {
	// HDR Middle Grey
	ID3D11Buffer* constBufferHDR = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::HDRStruct));
	{
		// HDR const buffer update and binding
		tre::HDRStruct hdrStruct = createHDRStruct(graphics.setting.middleGrey);
		tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), constBufferHDR, &hdrStruct, (UINT)sizeof(tre::HDRStruct));
		pEngine->device->context.Get()->PSSetConstantBuffers(4u, 1u, &constBufferHDR);
	}
	graphics.bufferQueue.push_back(constBufferHDR);
}

void RendererTonemap::fullscreenPass(const Graphics& graphics) {
	const char* name = ToString(RENDER_MODE::TONE_MAPPING_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Tone Mapping Fullscreen Pass");

	// Context Confiuration
	{
		pEngine->device->context.Get()->IASetInputLayout(nullptr);
		pEngine->device->context.Get()->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		pEngine->device->context.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		pEngine->device->context.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		pEngine->device->context.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		pEngine->device->context.Get()->PSSetShader(_hdrPixelShader.pShader.Get(), NULL, 0u);
		pEngine->device->context.Get()->PSSetShaderResources(0u, 1u, graphics._hdrBuffer.pShaderResViewHdrTexture.GetAddressOf()); // hdr texture
		pEngine->device->context.Get()->PSSetShaderResources(1u, 1u, graphics._hdrBuffer.pLuminAvgSRV.GetAddressOf());

		pEngine->device->context.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		pEngine->device->context.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		pEngine->device->context.Get()->OMSetRenderTargets(1, graphics.currRenderTargetView.GetAddressOf(), nullptr);
	}

	pEngine->device->context.Get()->Draw(6, 0);
}

void RendererTonemap::render(Graphics& graphics) {
	// Tone Mapping
	{
		PROFILE_GPU_SCOPED("HDR");
		setConstBufferHDR(graphics);
		fullscreenPass(graphics);
	}
}

}