#include "shadowmap.h"

#include "dxdebug.h"

namespace tre {

ShadowMap::ShadowMap(ID3D11Device* device) {
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
	shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	shadowMapDesc.Height = static_cast<UINT>(1024);
	shadowMapDesc.Width = static_cast<UINT>(1024);

	CHECK_DX_ERROR(device->CreateTexture2D(
		&shadowMapDesc, nullptr, shadowMap.GetAddressOf()
	));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	CHECK_DX_ERROR(device->CreateDepthStencilView(
		shadowMap.Get(), &depthStencilViewDesc, shadowDepthStencilView.GetAddressOf()
	));
	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		shadowMap.Get(), &shaderResourceViewDesc, shadowShaderRescView.GetAddressOf()
	));

}


}