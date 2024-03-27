#include "hdr.h"

namespace tre {

void HdrBuffer::create(ID3D11Device* device) {

	_device = device;

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

}
}