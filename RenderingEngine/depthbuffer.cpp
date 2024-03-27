#include "depthbuffer.h"

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

	// For shadow
	depthStencilDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.Height = static_cast<UINT>(4096);
	depthStencilDesc.Width = static_cast<UINT>(4096);
	depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS;

	CHECK_DX_ERROR(device->CreateTexture2D(
		&depthStencilDesc, nullptr, pShadowMapTexture.GetAddressOf()
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