#include "rendererHDR.h"

#include "utility.h"
#include "window.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

RendererHDR::RendererHDR() {
	this->init();
}

void RendererHDR::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_computeShaderLuminancehistogram.create(basePathWstr + L"shaders\\bin\\compute_shader_lumin_histogram.bin", pEngine->device->device.Get());
	_computeShaderLuminanceAverage.create(basePathWstr + L"shaders\\bin\\compute_shader_lumin_average.bin", pEngine->device->device.Get());
	
	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", pEngine->device->device.Get());
	_hdrPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_hdr_rendering.bin", pEngine->device->device.Get());
}

HDRStruct RendererHDR::createHDRStruct(float middleGrey) {
	HDRStruct hdrStruct;
	hdrStruct.middleGrey = middleGrey;

	return hdrStruct;
}

LuminanceStruct RendererHDR::createLuminanceStruct(const XMFLOAT2& luminance, float timeCoeff) {
	LuminanceStruct constBufferLumin;
	constBufferLumin.luminance = luminance;
	constBufferLumin.timeCoeff = timeCoeff;
	constBufferLumin.numPixel = SCREEN_HEIGHT * SCREEN_WIDTH;
	constBufferLumin.viewportDimension = XMINT2(SCREEN_WIDTH, SCREEN_HEIGHT);

	return constBufferLumin;
}

void RendererHDR::setConstBufferLuminSetting(Graphics& graphics) {
	// Luminance Setting
	ID3D11Buffer* constBufferLuminanceSetting = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::LuminanceStruct));
	{
		// Luminance Histogram Const Buffer update and binding
		tre::LuminanceStruct luminStruct = createLuminanceStruct(XMFLOAT2(graphics.setting.luminaceMin, graphics.setting.luminanceMax), graphics.setting.timeCoeff);
		tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), constBufferLuminanceSetting, &luminStruct, (UINT)sizeof(tre::LuminanceStruct));
		pEngine->device->context.Get()->CSSetConstantBuffers(0u, 1u, &constBufferLuminanceSetting);
	}
	graphics.bufferQueue.push_back(constBufferLuminanceSetting);
}

void RendererHDR::setConstBufferHDR(Graphics& graphics) {
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

void RendererHDR::dispatchHistogram(const Graphics& graphics){
	pEngine->device->context.Get()->CSSetShader(_computeShaderLuminancehistogram.pShader.Get(), NULL, 0u);
	pEngine->device->context.Get()->CSSetShaderResources(0u, 1u, graphics._hdrBuffer.pShaderResViewHdrTexture.GetAddressOf());
	pEngine->device->context.Get()->CSSetUnorderedAccessViews(0u, 1u, graphics._hdrBuffer.pLuminHistogramUAV.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Compute Shader Luminace Histogram");
		pEngine->device->context.Get()->Dispatch(tre::Maths::divideAndRoundUp(SCREEN_WIDTH, 16u), tre::Maths::divideAndRoundUp(SCREEN_HEIGHT, 16u), 1u);
	}
	pEngine->device->context.Get()->CSSetShaderResources(0u, 1u, graphics.nullSRV);
	pEngine->device->context.Get()->CSSetUnorderedAccessViews(0u, 1u, graphics.nullUAV, nullptr);
}

void RendererHDR::dispatchAverage(const Graphics& graphics){
	pEngine->device->context.Get()->CSSetShader(_computeShaderLuminanceAverage.pShader.Get(), NULL, 0u);
	pEngine->device->context.Get()->CSSetUnorderedAccessViews(0u, 1, graphics._hdrBuffer.pLuminHistogramUAV.GetAddressOf(), nullptr);
	pEngine->device->context.Get()->CSSetUnorderedAccessViews(1u, 1, graphics._hdrBuffer.pLuminAvgUAV.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Compute Shader Luminace Average");
		pEngine->device->context.Get()->Dispatch(1u, 1u, 1u);
	}
	pEngine->device->context.Get()->CSSetUnorderedAccessViews(0u, 1, graphics.nullUAV, nullptr);
	pEngine->device->context.Get()->CSSetUnorderedAccessViews(1u, 1, graphics.nullUAV, nullptr);
}

void RendererHDR::fullscreenPass(const Graphics& graphics) {
	const char* name = ToString(RENDER_MODE::TONE_MAPPING_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("HDR Fullscreen Pass");
	
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

void RendererHDR::render(Graphics& graphics) {
	pEngine->device->context.Get()->OMSetRenderTargets(0, nullptr, nullptr);
	// Luminance Histogram
	{
		PROFILE_GPU_SCOPED("CS: Luminance Histogram");
		setConstBufferLuminSetting(graphics);
		dispatchHistogram(graphics);
		dispatchAverage(graphics);
	}

	// Tone Mapping
	{
		PROFILE_GPU_SCOPED("HDR");
		setConstBufferHDR(graphics);
		fullscreenPass(graphics);
	}
}

}