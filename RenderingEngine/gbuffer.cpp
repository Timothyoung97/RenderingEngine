#include "gbuffer.h"
#include "window.h"
#include "dxdebug.h"

namespace tre {
void GBuffer::create(ID3D11Device* device) {
	D3D11_TEXTURE2D_DESC gbufferDesc;
	gbufferDesc.Width = SCREEN_WIDTH;
	gbufferDesc.Height = SCREEN_HEIGHT;
	gbufferDesc.MipLevels = 1;
	gbufferDesc.ArraySize = 1;
	gbufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	gbufferDesc.SampleDesc.Count = 1;
	gbufferDesc.SampleDesc.Quality = 0;
	gbufferDesc.Usage = D3D11_USAGE_DEFAULT; // GPU Read and Write
	gbufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // Render to buffer, then use as shader resource
	gbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	gbufferDesc.MiscFlags = 0;

	CHECK_DX_ERROR(device->CreateTexture2D(
		&gbufferDesc, nullptr, pGBufferTextureAlbedo.GetAddressOf()
	));

	CHECK_DX_ERROR(device->CreateTexture2D(
		&gbufferDesc, nullptr, pGBufferTextureNormal.GetAddressOf()
	));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
	shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, -1);

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		pGBufferTextureAlbedo.Get(), &shaderResViewDesc, pShaderResViewDeferredAlbedo.GetAddressOf()
	));

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		pGBufferTextureNormal.Get(), &shaderResViewDesc, pShaderResViewDeferredNormal.GetAddressOf()
	));

	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	ZeroMemory(&rtvd, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvd.Texture2D.MipSlice = 0;

	CHECK_DX_ERROR(device->CreateRenderTargetView(
		pGBufferTextureAlbedo.Get(), &rtvd, pRenderTargetViewDeferredAlbedo.GetAddressOf()
	));

	CHECK_DX_ERROR(device->CreateRenderTargetView(
		pGBufferTextureNormal.Get(), &rtvd, pRenderTargetViewDeferredNormal.GetAddressOf()
	));

	rtvs[0] = pRenderTargetViewDeferredAlbedo.Get(), rtvs[1] = pRenderTargetViewDeferredNormal.Get();
}
}