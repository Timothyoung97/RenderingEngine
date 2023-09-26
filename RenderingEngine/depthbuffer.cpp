#include "depthbuffer.h"
#include "dxdebug.h"

namespace tre {

void DepthBuffer::create(ID3D11Device* device, int screenW, int screenH) {
	
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = screenW;
	depthStencilDesc.Height = screenH;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	CHECK_DX_ERROR(device->CreateTexture2D(
		&depthStencilDesc, nullptr, pDepthStencilTexture.GetAddressOf()
	));

	CHECK_DX_ERROR(device->CreateDepthStencilView(
		pDepthStencilTexture.Get(), nullptr, pDepthStencilView.GetAddressOf()
	));

	// For shadow
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.Height = static_cast<UINT>(4096);
	depthStencilDesc.Width = static_cast<UINT>(4096);

	CHECK_DX_ERROR(device->CreateTexture2D(
		&depthStencilDesc, nullptr, pShadowMapTexture.GetAddressOf()
	));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	CHECK_DX_ERROR(device->CreateDepthStencilView(
		pShadowMapTexture.Get(), &depthStencilViewDesc, pShadowDepthStencilView.GetAddressOf()
	));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

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

	// stencil operations if pixel is front-facing
	ddsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ddsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	ddsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	ddsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;	
	
	// stencil operations if pixel is back-facing
	ddsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ddsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	ddsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	ddsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

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