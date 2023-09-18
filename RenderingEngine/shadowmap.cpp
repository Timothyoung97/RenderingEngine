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

	D3D11_SAMPLER_DESC shadowSamplerDesc;
	ZeroMemory(&shadowSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	shadowSamplerDesc.MinLOD = 0.f;
	shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	shadowSamplerDesc.MipLODBias = 0.f;
	shadowSamplerDesc.MaxAnisotropy = 0;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

	CHECK_DX_ERROR(device->CreateSamplerState(&shadowSamplerDesc, shadowSamplerState.GetAddressOf()));

	D3D11_RASTERIZER_DESC drawingRenderStateDesc;
	ZeroMemory(&drawingRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	drawingRenderStateDesc.CullMode = D3D11_CULL_BACK;
	drawingRenderStateDesc.FillMode = D3D11_FILL_SOLID;
	drawingRenderStateDesc.DepthClipEnable = true; // Feature level 9_1 requires DepthClipEnable == true
	CHECK_DX_ERROR(device->CreateRasterizerState(&drawingRenderStateDesc, drawingRasterizerState.GetAddressOf()));


	D3D11_RASTERIZER_DESC shadowRenderStateDesc;
	ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	shadowRenderStateDesc.CullMode = D3D11_CULL_FRONT;
	shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
	shadowRenderStateDesc.DepthClipEnable = true; 
	
	CHECK_DX_ERROR(device->CreateRasterizerState(&shadowRenderStateDesc, shadowRasterizerState.GetAddressOf()));

	// Init viewport for shadow rendering
	ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
	shadowViewport.Height = 1024;
	shadowViewport.Width = 1024;
	shadowViewport.MinDepth = 0.f;
	shadowViewport.MaxDepth = 1.f;
	
}


}