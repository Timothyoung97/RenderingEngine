#include "computerHDR.h"

#include "utility.h"
#include "window.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

ComputerHDR::ComputerHDR() {
	this->init();
}

void ComputerHDR::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_computeShaderLuminancehistogram.create(basePathWstr + L"shaders\\bin\\compute_shader_lumin_histogram.bin", pEngine->device->device.Get());
	_computeShaderLuminanceAverage.create(basePathWstr + L"shaders\\bin\\compute_shader_lumin_average.bin", pEngine->device->device.Get());
}

LuminanceStruct ComputerHDR::createLuminanceStruct(const XMFLOAT2& luminance, float timeCoeff) {
	LuminanceStruct constBufferLumin;
	constBufferLumin.log2luminance = XMFLOAT2(log2f(luminance.x), log2f(luminance.y));
	constBufferLumin.timeCoeff = timeCoeff;
	constBufferLumin.numPixel = SCREEN_HEIGHT * SCREEN_WIDTH;
	constBufferLumin.viewportDimension = XMINT2(SCREEN_WIDTH, SCREEN_HEIGHT);

	return constBufferLumin;
}

void ComputerHDR::setConstBufferLuminSetting(Graphics& graphics) {
	// Luminance Setting
	ID3D11Buffer* constBufferLuminanceSetting = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::LuminanceStruct));
	{
		// Luminance Histogram Const Buffer update and binding
		tre::LuminanceStruct luminStruct = createLuminanceStruct(XMFLOAT2(graphics.setting.luminaceMin, graphics.setting.luminanceMax), graphics.setting.timeCoeff);
		tre::Buffer::updateConstBufferData(pEngine->device->contextI.Get(), constBufferLuminanceSetting, &luminStruct, (UINT)sizeof(tre::LuminanceStruct));
		pEngine->device->contextI.Get()->CSSetConstantBuffers(0u, 1u, &constBufferLuminanceSetting);
	}
	graphics.bufferQueue.push_back(constBufferLuminanceSetting);
}

void ComputerHDR::dispatchHistogram(const Graphics& graphics){
	pEngine->device->contextI.Get()->CSSetShader(_computeShaderLuminancehistogram.pShader.Get(), NULL, 0u);
	pEngine->device->contextI.Get()->CSSetShaderResources(0u, 1u, graphics._hdrBuffer.pShaderResViewHdrTexture.GetAddressOf());
	pEngine->device->contextI.Get()->CSSetUnorderedAccessViews(0u, 1u, graphics._hdrBuffer.pLuminHistogramUAV.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Compute Shader Luminace Histogram");
		pEngine->device->contextI.Get()->Dispatch(tre::Maths::divideAndRoundUp(SCREEN_WIDTH, 16u), tre::Maths::divideAndRoundUp(SCREEN_HEIGHT, 16u), 1u);
	}
	pEngine->device->contextI.Get()->CSSetShaderResources(0u, 1u, graphics.nullSRV);
	pEngine->device->contextI.Get()->CSSetUnorderedAccessViews(0u, 1u, graphics.nullUAV, nullptr);
}

void ComputerHDR::dispatchAverage(const Graphics& graphics){
	pEngine->device->contextI.Get()->CSSetShader(_computeShaderLuminanceAverage.pShader.Get(), NULL, 0u);
	pEngine->device->contextI.Get()->CSSetUnorderedAccessViews(0u, 1, graphics._hdrBuffer.pLuminHistogramUAV.GetAddressOf(), nullptr);
	pEngine->device->contextI.Get()->CSSetUnorderedAccessViews(1u, 1, graphics._hdrBuffer.pLuminAvgUAV.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Compute Shader Luminace Average");
		pEngine->device->contextI.Get()->Dispatch(1u, 1u, 1u);
	}
	pEngine->device->contextI.Get()->CSSetUnorderedAccessViews(0u, 1, graphics.nullUAV, nullptr);
	pEngine->device->contextI.Get()->CSSetUnorderedAccessViews(1u, 1, graphics.nullUAV, nullptr);
}

void ComputerHDR::compute(Graphics& graphics) {
	pEngine->device->contextI.Get()->OMSetRenderTargets(0, nullptr, nullptr);
	// Luminance Histogram
	PROFILE_GPU_SCOPED("HDR Compute");
	PROFILE_GPU_SCOPED("HDR Compute");
	setConstBufferLuminSetting(graphics);
	dispatchHistogram(graphics);
	dispatchAverage(graphics);
}
}