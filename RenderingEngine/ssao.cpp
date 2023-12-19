#include <DirectXMath.h>

#include <vector>

#include "ssao.h"
#include "utility.h"
#include "dxdebug.h"
#include "maths.h"
#include "window.h"

using namespace DirectX;

namespace tre {

void SSAO::create(ID3D11Device* device) {
	_device = device;

	// create ssaoKernal, to be sent in as const buffer
	for (int i = 0; i < 64; i++) {
		ssaoKernalSamples.push_back(XMFLOAT4(tre::Utility::getRandomFloatRange(-75.f, 75.f), .0f, .0f, .0f));
	}

	// create ssaoResultTexture2d, to be used as render result
	D3D11_TEXTURE2D_DESC ssaoResultTexture2dDesc;
	ssaoResultTexture2dDesc.Width = SCREEN_WIDTH;
	ssaoResultTexture2dDesc.Height = SCREEN_HEIGHT;
	ssaoResultTexture2dDesc.MipLevels = 1;
	ssaoResultTexture2dDesc.ArraySize = 1;
	ssaoResultTexture2dDesc.Format = DXGI_FORMAT_R8_UNORM; // DXGI_FORMAT_R8_UNORM
	ssaoResultTexture2dDesc.SampleDesc.Count = 1;
	ssaoResultTexture2dDesc.SampleDesc.Quality = 0;
	ssaoResultTexture2dDesc.Usage = D3D11_USAGE_DEFAULT; // GPU Read and Write
	ssaoResultTexture2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // Render to buffer, then use as shader resource
	ssaoResultTexture2dDesc.CPUAccessFlags = 0u;
	ssaoResultTexture2dDesc.MiscFlags = 0;

	CHECK_DX_ERROR(_device->CreateTexture2D(
		&ssaoResultTexture2dDesc, nullptr, ssaoResultTexture2d.GetAddressOf()
	));

	CHECK_DX_ERROR(_device->CreateTexture2D(
		&ssaoResultTexture2dDesc, nullptr, ssaoBlurredTexture2d.GetAddressOf()
	));

	D3D11_SHADER_RESOURCE_VIEW_DESC ssaoResultTexture2dSRVDesc;
	ssaoResultTexture2dSRVDesc.Format = DXGI_FORMAT_R8_UNORM;
	ssaoResultTexture2dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ssaoResultTexture2dSRVDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		ssaoResultTexture2d.Get(), &ssaoResultTexture2dSRVDesc, ssaoResultTexture2dSRV.GetAddressOf()
	));
	
	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		ssaoBlurredTexture2d.Get(), &ssaoResultTexture2dSRVDesc, ssaoBlurredTexture2dSRV.GetAddressOf()
	));

	D3D11_RENDER_TARGET_VIEW_DESC ssaoResultTexture2dRTVDesc;
	ZeroMemory(&ssaoResultTexture2dRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	ssaoResultTexture2dRTVDesc.Format = DXGI_FORMAT_R8_UNORM;
	ssaoResultTexture2dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	ssaoResultTexture2dRTVDesc.Texture2D.MipSlice = 0;

	CHECK_DX_ERROR(_device->CreateRenderTargetView(
		ssaoResultTexture2d.Get(), &ssaoResultTexture2dRTVDesc, ssaoResultTexture2dRTV.GetAddressOf()
	));

	CHECK_DX_ERROR(_device->CreateRenderTargetView(
		ssaoBlurredTexture2d.Get(), &ssaoResultTexture2dRTVDesc, ssaoBlurredTexture2dRTV.GetAddressOf()
	));
}
}