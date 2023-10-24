#include "hdr.h"

#include "window.h"
#include "dxdebug.h"

namespace tre {

void HdrBuffer::create(ID3D11Device* device) {
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

	CHECK_DX_ERROR(device->CreateTexture2D(
		&hdrBufferDesc, nullptr, pHdrBufferTexture.GetAddressOf()
	));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
	shaderResViewDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		pHdrBufferTexture.Get(), &shaderResViewDesc, pShaderResViewHdrTexture.GetAddressOf()
	));

	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	ZeroMemory(&rtvd, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvd.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvd.Texture2D.MipSlice = 0;

	CHECK_DX_ERROR(device->CreateRenderTargetView(
		pHdrBufferTexture.Get(), &rtvd, pRenderTargetViewHdrTexture.GetAddressOf()
	));
}
}