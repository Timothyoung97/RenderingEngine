#include "rendererHDR.h"
#include "utility.h"
#include "window.h"

namespace tre {

RendererHDR::RendererHDR(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_computeShaderLuminancehistogram.create(basePathWstr + L"shaders\\bin\\compute_shader_lumin_histogram.bin", _device);
	_computeShaderLuminanceAverage.create(basePathWstr + L"shaders\\bin\\compute_shader_lumin_average.bin", _device);
	
	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", _device);
	_hdrPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_hdr_rendering.bin", _device);
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
	ID3D11Buffer* constBufferLuminanceSetting = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::LuminanceStruct));
	{
		// Luminance Histogram Const Buffer update and binding
		tre::LuminanceStruct luminStruct = createLuminanceStruct(XMFLOAT2(graphics.setting.luminaceMin, graphics.setting.luminanceMax), graphics.setting.timeCoeff);
		tre::Buffer::updateConstBufferData(_context, constBufferLuminanceSetting, &luminStruct, (UINT)sizeof(tre::LuminanceStruct));
		_context->CSSetConstantBuffers(0u, 1u, &constBufferLuminanceSetting);
	}
	graphics.bufferQueue.push_back(constBufferLuminanceSetting);
}

void RendererHDR::setConstBufferHDR(Graphics& graphics) {
	// HDR Middle Grey
	ID3D11Buffer* constBufferHDR = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::HDRStruct));
	{
		// HDR const buffer update and binding
		tre::HDRStruct hdrStruct = createHDRStruct(graphics.setting.middleGrey);
		tre::Buffer::updateConstBufferData(_context, constBufferHDR, &hdrStruct, (UINT)sizeof(tre::HDRStruct));
		_context->PSSetConstantBuffers(4u, 1u, &constBufferHDR);
	}
	graphics.bufferQueue.push_back(constBufferHDR);
}

void RendererHDR::dispatchHistogram(const Graphics& graphics){
	_context->CSSetShader(_computeShaderLuminancehistogram.pShader.Get(), NULL, 0u);
	_context->CSSetShaderResources(0u, 1u, graphics._hdrBuffer.pShaderResViewHdrTexture.GetAddressOf());
	_context->CSSetUnorderedAccessViews(0u, 1u, graphics._hdrBuffer.pLuminHistogramUAV.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Compute Shader Luminace Histogram");
		_context->Dispatch(tre::Maths::divideAndRoundUp(SCREEN_WIDTH, 16u), tre::Maths::divideAndRoundUp(SCREEN_HEIGHT, 16u), 1u);
	}
	_context->CSSetShaderResources(0u, 1u, graphics.nullSRV);
	_context->CSSetUnorderedAccessViews(0u, 1u, graphics.nullUAV, nullptr);
}

void RendererHDR::dispatchAverage(const Graphics& graphics){
	_context->CSSetShader(_computeShaderLuminanceAverage.pShader.Get(), NULL, 0u);
	_context->CSSetUnorderedAccessViews(0u, 1, graphics._hdrBuffer.pLuminHistogramUAV.GetAddressOf(), nullptr);
	_context->CSSetUnorderedAccessViews(1u, 1, graphics._hdrBuffer.pLuminAvgUAV.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Compute Shader Luminace Average");
		_context->Dispatch(1u, 1u, 1u);
	}
	_context->CSSetUnorderedAccessViews(0u, 1, graphics.nullUAV, nullptr);
	_context->CSSetUnorderedAccessViews(1u, 1, graphics.nullUAV, nullptr);
}

void RendererHDR::fullscreenPass(const Graphics& graphics) {
	const char* name = ToString(RENDER_MODE::TONE_MAPPING_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Fullscreen Pass");
	
	// Context Confiuration
	{
		_context->IASetInputLayout(nullptr);
		_context->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_hdrPixelShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(0u, 1u, graphics._hdrBuffer.pShaderResViewHdrTexture.GetAddressOf()); // hdr texture
		_context->PSSetShaderResources(1u, 1u, graphics._hdrBuffer.pLuminAvgSRV.GetAddressOf());

		_context->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		_context->OMSetRenderTargets(1, &graphics.currRenderTargetView, nullptr);
	}

	_context->Draw(6, 0);
}

void RendererHDR::render(Graphics& graphics) {
	_context->OMSetRenderTargets(0, nullptr, nullptr);
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