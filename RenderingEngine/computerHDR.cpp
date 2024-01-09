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
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferLuminanceSetting, &luminStruct, (UINT)sizeof(tre::LuminanceStruct));
		contextD.Get()->CSSetConstantBuffers(0u, 1u, &constBufferLuminanceSetting);
	}

	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferLuminanceSetting);
	}
}

void ComputerHDR::dispatchHistogram(const Graphics& graphics){

	// Create views
	ComPtr<ID3D11ShaderResourceView> shaderResViewHdrTexture;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
		shaderResViewDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._hdrBuffer.pHdrBufferTexture.Get(), &shaderResViewDesc, shaderResViewHdrTexture.GetAddressOf()
		));
	}

	ComPtr<ID3D11UnorderedAccessView> luminHistogramUAV;
	{
		D3D11_BUFFER_UAV pLuminHistogramBufferUAV;
		pLuminHistogramBufferUAV.NumElements = 256;
		pLuminHistogramBufferUAV.FirstElement = 0;
		pLuminHistogramBufferUAV.Flags = 0u;

		D3D11_UNORDERED_ACCESS_VIEW_DESC pLuminHistogramUAVDesc;
		pLuminHistogramUAVDesc.Format = DXGI_FORMAT_R32_UINT;
		pLuminHistogramUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		pLuminHistogramUAVDesc.Buffer = pLuminHistogramBufferUAV;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateUnorderedAccessView(
			graphics._hdrBuffer.pLuminHistogram.Get(), &pLuminHistogramUAVDesc, luminHistogramUAV.GetAddressOf()
		));
	}

	contextD.Get()->CSSetShader(_computeShaderLuminancehistogram.pShader.Get(), NULL, 0u);
	contextD.Get()->CSSetShaderResources(0u, 1u, shaderResViewHdrTexture.GetAddressOf());
	contextD.Get()->CSSetUnorderedAccessViews(0u, 1u, luminHistogramUAV.GetAddressOf(), nullptr);
	{
		//PROFILE_GPU_SCOPED("Compute Shader Luminace Histogram");
		contextD.Get()->Dispatch(tre::Maths::divideAndRoundUp(SCREEN_WIDTH, 16u), tre::Maths::divideAndRoundUp(SCREEN_HEIGHT, 16u), 1u);
	}
	contextD.Get()->CSSetShaderResources(0u, 1u, graphics.nullSRV);
	contextD.Get()->CSSetUnorderedAccessViews(0u, 1u, graphics.nullUAV, nullptr);
}

void ComputerHDR::dispatchAverage(const Graphics& graphics){

	ComPtr<ID3D11UnorderedAccessView> luminHistogramUAV;
	{
		D3D11_BUFFER_UAV pLuminHistogramBufferUAV;
		pLuminHistogramBufferUAV.NumElements = 256;
		pLuminHistogramBufferUAV.FirstElement = 0;
		pLuminHistogramBufferUAV.Flags = 0u;

		D3D11_UNORDERED_ACCESS_VIEW_DESC pLuminHistogramUAVDesc;
		pLuminHistogramUAVDesc.Format = DXGI_FORMAT_R32_UINT;
		pLuminHistogramUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		pLuminHistogramUAVDesc.Buffer = pLuminHistogramBufferUAV;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateUnorderedAccessView(
			graphics._hdrBuffer.pLuminHistogram.Get(), &pLuminHistogramUAVDesc, luminHistogramUAV.GetAddressOf()
		));
	}

	ComPtr<ID3D11UnorderedAccessView> luminAvgUAV;
	{
		D3D11_BUFFER_UAV pLuminAvgBufferUAV;
		pLuminAvgBufferUAV.NumElements = 1;
		pLuminAvgBufferUAV.FirstElement = 0;
		pLuminAvgBufferUAV.Flags = 0u;

		D3D11_UNORDERED_ACCESS_VIEW_DESC pLuminAvgUAVDesc;
		pLuminAvgUAVDesc.Format = DXGI_FORMAT_R16_FLOAT;
		pLuminAvgUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		pLuminAvgUAVDesc.Buffer = pLuminAvgBufferUAV;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateUnorderedAccessView(
			graphics._hdrBuffer.pLuminAvg.Get(), &pLuminAvgUAVDesc, luminAvgUAV.GetAddressOf()
		));
	}

	contextD.Get()->CSSetShader(_computeShaderLuminanceAverage.pShader.Get(), NULL, 0u);
	contextD.Get()->CSSetUnorderedAccessViews(0u, 1, luminHistogramUAV.GetAddressOf(), nullptr);
	contextD.Get()->CSSetUnorderedAccessViews(1u, 1, luminAvgUAV.GetAddressOf(), nullptr);
	{
		//PROFILE_GPU_SCOPED("Compute Shader Luminace Average");
		contextD.Get()->Dispatch(1u, 1u, 1u);
	}
	contextD.Get()->CSSetUnorderedAccessViews(0u, 1, graphics.nullUAV, nullptr);
	contextD.Get()->CSSetUnorderedAccessViews(1u, 1, graphics.nullUAV, nullptr);
}

void ComputerHDR::compute(Graphics& graphics) {
	contextD.Get()->OMSetRenderTargets(0, nullptr, nullptr);
	// Luminance Histogram
	//PROFILE_GPU_SCOPED("HDR Compute");
	setConstBufferLuminSetting(graphics);
	dispatchHistogram(graphics);
	dispatchAverage(graphics);

	CHECK_DX_ERROR(contextD.Get()->FinishCommandList(
		false, &commandList
	));
}
}