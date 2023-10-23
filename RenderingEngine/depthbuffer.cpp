#include "depthbuffer.h"
#include "dxdebug.h"

namespace tre {

void DepthBuffer::create(ID3D11Device* device, int screenW, int screenH) {
	
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = screenW;
	depthStencilDesc.Height = screenH;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	CHECK_DX_ERROR(device->CreateTexture2D(
		&depthStencilDesc, nullptr, pDepthStencilTexture.GetAddressOf()
	));

	CHECK_DX_ERROR(device->CreateTexture2D(
		&depthStencilDesc, nullptr, pDepthStencilReadOnlyTexture.GetAddressOf()
	));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	CHECK_DX_ERROR(device->CreateDepthStencilView(
		pDepthStencilTexture.Get(), &depthStencilViewDesc, pDepthStencilView.GetAddressOf()
	));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		pDepthStencilTexture.Get(), &shaderResourceViewDesc, pDepthStencilShaderRescView.GetAddressOf()
	));	
	
	CHECK_DX_ERROR(device->CreateShaderResourceView(
		pDepthStencilReadOnlyTexture.Get(), &shaderResourceViewDesc, pDepthStencilReadOnlyShaderRescView.GetAddressOf()
	));

	// For shadow
	depthStencilDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.Height = static_cast<UINT>(4096);
	depthStencilDesc.Width = static_cast<UINT>(4096);

	CHECK_DX_ERROR(device->CreateTexture2D(
		&depthStencilDesc, nullptr, pShadowMapTexture.GetAddressOf()
	));

	CHECK_DX_ERROR(device->CreateDepthStencilView(
		pShadowMapTexture.Get(), &depthStencilViewDesc, pShadowDepthStencilView.GetAddressOf()
	));

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		pShadowMapTexture.Get(), &shaderResourceViewDesc, pShadowShaderRescView.GetAddressOf()
	));

	D3D11_DEPTH_STENCIL_DESC ddsd;
	
	// Depth test
	ddsd.DepthEnable = false;
	ddsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	ddsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
	
	// stencil test parameters
	ddsd.StencilEnable = false;
	ddsd.StencilReadMask = 0xFF;
	ddsd.StencilWriteMask = 0xFF;

	CHECK_DX_ERROR(device->CreateDepthStencilState(
		&ddsd, pDSStateWithoutDepthT.GetAddressOf()
	));

	// Depth test Enable without write mask
	ddsd.DepthEnable = true;
	ddsd.DepthFunc = D3D11_COMPARISON_LESS;

	CHECK_DX_ERROR(device->CreateDepthStencilState(
		&ddsd, pDSStateWithDepthTWriteDisabled.GetAddressOf()
	));

	// Depth test Enable 
	ddsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	CHECK_DX_ERROR(device->CreateDepthStencilState(
		&ddsd, pDSStateWithDepthTWriteEnabled.GetAddressOf()
	));

}
}