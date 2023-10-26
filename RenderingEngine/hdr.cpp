#include "hdr.h"

#include "window.h"
#include "dxdebug.h"
#include "utility.h"
#include "maths.h"

namespace tre {

void HdrBuffer::create(ID3D11Device* device, ID3D11DeviceContext* context) {

	_device = device;
	_context = context;

	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	computeShaderLuminancehistogram.create(basePathWstr + L"shaders\\bin\\compute_shader_lumin_histogram.bin", _device);
	computeShaderLuminanceAverage.create(basePathWstr + L"shaders\\bin\\compute_shader_lumin_average.bin", _device);
	
	// HDR Texture to be rendered
	D3D11_TEXTURE2D_DESC hdrBufferDesc;
	hdrBufferDesc.Width = SCREEN_WIDTH;
	hdrBufferDesc.Height = SCREEN_HEIGHT;
	hdrBufferDesc.MipLevels = 1;
	hdrBufferDesc.ArraySize = 1;
	hdrBufferDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	hdrBufferDesc.SampleDesc.Count = 1;
	hdrBufferDesc.SampleDesc.Quality = 0;
	hdrBufferDesc.Usage = D3D11_USAGE_DEFAULT; // GPU Read and Write
	hdrBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // Render to buffer, then use as shader resource
	hdrBufferDesc.CPUAccessFlags = 0u;
	hdrBufferDesc.MiscFlags = 0;
	
	CHECK_DX_ERROR(_device->CreateTexture2D(
		&hdrBufferDesc, nullptr, pHdrBufferTexture.GetAddressOf()
	));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
	shaderResViewDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);
	
	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		pHdrBufferTexture.Get(), &shaderResViewDesc, pShaderResViewHdrTexture.GetAddressOf()
	));

	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	ZeroMemory(&rtvd, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvd.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvd.Texture2D.MipSlice = 0;
	
	CHECK_DX_ERROR(_device->CreateRenderTargetView(
		pHdrBufferTexture.Get(), &rtvd, pRenderTargetViewHdrTexture.GetAddressOf()
	));

	// Luminance Histogram
	D3D11_BUFFER_DESC pLuminHistogramBufferDesc;
	pLuminHistogramBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	pLuminHistogramBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	pLuminHistogramBufferDesc.CPUAccessFlags = 0u;
	pLuminHistogramBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	pLuminHistogramBufferDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * 256);
	pLuminHistogramBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(UINT));

	int defaultHistogramValue[256] = { 0 };
	D3D11_SUBRESOURCE_DATA histogramData;
	histogramData.pSysMem = &defaultHistogramValue;

	CHECK_DX_ERROR(_device->CreateBuffer(
		&pLuminHistogramBufferDesc, &histogramData, pLuminHistogram.GetAddressOf()
	));

	D3D11_BUFFER_UAV pLuminHistogramBufferUAV;
	pLuminHistogramBufferUAV.NumElements = 256;
	pLuminHistogramBufferUAV.FirstElement = 0;
	pLuminHistogramBufferUAV.Flags = 0u;

	D3D11_UNORDERED_ACCESS_VIEW_DESC pLuminHistogramUAVDesc;
	pLuminHistogramUAVDesc.Format = DXGI_FORMAT_R32_UINT;
	pLuminHistogramUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	pLuminHistogramUAVDesc.Buffer = pLuminHistogramBufferUAV;

	CHECK_DX_ERROR(_device->CreateUnorderedAccessView(
		pLuminHistogram.Get(), &pLuminHistogramUAVDesc, pLuminHistogramUAV.GetAddressOf()
	));

	// Luminance Average
	D3D11_BUFFER_DESC pLuminAvgBufferDesc;
	pLuminAvgBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	pLuminAvgBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	pLuminAvgBufferDesc.CPUAccessFlags = 0u;
	pLuminAvgBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	pLuminAvgBufferDesc.ByteWidth = static_cast<UINT>(sizeof(UINT));
	pLuminAvgBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(UINT));

	float defaultLuminAvg = 0.f;
	D3D11_SUBRESOURCE_DATA luminAvgData;
	luminAvgData.pSysMem = &defaultLuminAvg;

	CHECK_DX_ERROR(_device->CreateBuffer(
		&pLuminAvgBufferDesc, &luminAvgData, pLuminAvg.GetAddressOf()
	));

	D3D11_BUFFER_SRV pLuminAvgBufferSRV;
	pLuminAvgBufferSRV.NumElements = 1;
	pLuminAvgBufferSRV.FirstElement = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC pLuminAvgSRVResc;
	pLuminAvgSRVResc.Format = DXGI_FORMAT_R16_FLOAT;
	pLuminAvgSRVResc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	pLuminAvgSRVResc.Buffer = pLuminAvgBufferSRV;

	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		pLuminAvg.Get(), &pLuminAvgSRVResc, pLuminAvgSRV.GetAddressOf()
	));

	D3D11_BUFFER_UAV pLuminAvgBufferUAV;
	pLuminAvgBufferUAV.NumElements = 1;
	pLuminAvgBufferUAV.FirstElement = 0;
	pLuminAvgBufferUAV.Flags = 0u;

	D3D11_UNORDERED_ACCESS_VIEW_DESC pLuminAvgUAVDesc;
	pLuminAvgUAVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	pLuminAvgUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	pLuminAvgUAVDesc.Buffer = pLuminAvgBufferUAV;

	CHECK_DX_ERROR(_device->CreateUnorderedAccessView(
		pLuminAvg.Get(), &pLuminAvgUAVDesc, pLuminAvgUAV.GetAddressOf()
	));
}

void HdrBuffer::dispatchHistogram() {
	_context->CSSetShader(computeShaderLuminancehistogram.pShader.Get(), NULL, 0u);
	_context->CSSetShaderResources(8u, 1u, pShaderResViewHdrTexture.GetAddressOf());
	_context->CSSetUnorderedAccessViews(0, 1, pLuminHistogramUAV.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Compute Shader Luminace Histogram");
		_context->Dispatch(tre::Maths::divideAndRoundUp(SCREEN_WIDTH, 16u), tre::Maths::divideAndRoundUp(SCREEN_HEIGHT, 16u), 1u);
	}
	_context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}

void HdrBuffer::dispatchAverage() {
	_context->CSSetShader(computeShaderLuminanceAverage.pShader.Get(), NULL, 0u);
	_context->CSSetUnorderedAccessViews(0u, 1, pLuminHistogramUAV.GetAddressOf(), nullptr);
	_context->CSSetUnorderedAccessViews(1u, 1, pLuminAvgUAV.GetAddressOf(), nullptr);
	{
		PROFILE_GPU_SCOPED("Compute Shader Luminace Average");
		_context->Dispatch(1u, 1u, 1u);
	}
	_context->CSSetUnorderedAccessViews(0u, 1, nullUAV, nullptr);
	_context->CSSetUnorderedAccessViews(1u, 1, nullUAV, nullptr);
}
}