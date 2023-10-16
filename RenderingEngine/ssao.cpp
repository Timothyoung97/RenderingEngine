#include <DirectXMath.h>

#include <vector>

#include "ssao.h"
#include "utility.h"
#include "dxdebug.h"
#include "maths.h"
#include "window.h"

using namespace DirectX;

namespace tre {

void SSAO::create(ID3D11Device* device, ID3D11DeviceContext* context) {
	_device = device;
	_context = context;

	// create ssaoKernal, to be sent in as const buffer
	for (int i = 0; i < 64; i++) {
		XMVECTOR newSample{
			tre::Utility::getRandomFloatRange(-1.f, 1.f),
			tre::Utility::getRandomFloatRange(-1.f, 1.f),
			tre::Utility::getRandomFloatRange( 0.f, 1.f)
		};

		newSample = XMVector3Normalize(newSample);

		float scale = (float)i / 64.0f;
		scale = tre::Maths::lerp(.1f, .1f, scale * scale);

		XMFLOAT3 sampleF;
		XMStoreFloat3(&sampleF, newSample * scale);

		ssaoKernalSamples.push_back(XMFLOAT4(sampleF.x, sampleF.y, sampleF.z, .0f));
	}

	// create ssaoNoise, to be used as texture2D
	std::vector<XMFLOAT4> ssaoNoise;
	for (int i = 0; i < 16; i++) {
		XMFLOAT4 newNoise = XMFLOAT4(
			tre::Utility::getRandomFloatRange(0, 1.f),
			tre::Utility::getRandomFloatRange(0, 1.f),
			.0f,
			.0f
		);
		ssaoNoise.push_back(newNoise);
	}

	D3D11_TEXTURE2D_DESC ssaoNoiseTextureDesc;
	ssaoNoiseTextureDesc.Width = 4;
	ssaoNoiseTextureDesc.Height = 4;
	ssaoNoiseTextureDesc.MipLevels = 1;
	ssaoNoiseTextureDesc.ArraySize = 1;
	ssaoNoiseTextureDesc.Format = DXGI_FORMAT_R8G8_UNORM; // DXGI_FORMAT_R8G8_UNORM
	ssaoNoiseTextureDesc.SampleDesc.Count = 1;
	ssaoNoiseTextureDesc.SampleDesc.Quality = 0;
	ssaoNoiseTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	ssaoNoiseTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ssaoNoiseTextureDesc.CPUAccessFlags = 0;
	ssaoNoiseTextureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA ssaoNoiseTextureData = {};
	ssaoNoiseTextureData.pSysMem = ssaoNoise.data();
	ssaoNoiseTextureData.SysMemPitch = static_cast<UINT>(4 * sizeof(XMFLOAT4));
	ssaoNoiseTextureData.SysMemSlicePitch = 0u;

	CHECK_DX_ERROR(_device->CreateTexture2D(
		&ssaoNoiseTextureDesc, &ssaoNoiseTextureData, ssaoNoiseTexture2d.GetAddressOf()
	));

	D3D11_SHADER_RESOURCE_VIEW_DESC ssaoNoiseSRVDesc;
	ssaoNoiseSRVDesc.Format = DXGI_FORMAT_R8G8_UNORM;
	ssaoNoiseSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ssaoNoiseSRVDesc.Texture2D = D3D11_TEX2D_SRV(0, -1);

	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		ssaoNoiseTexture2d.Get(), &ssaoNoiseSRVDesc, ssaoNoiseTexture2dSRV.GetAddressOf()
	));

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

	D3D11_SHADER_RESOURCE_VIEW_DESC ssaoResultTexture2dSRVDesc;
	ssaoResultTexture2dSRVDesc.Format = DXGI_FORMAT_R8_UNORM;
	ssaoResultTexture2dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ssaoResultTexture2dSRVDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		ssaoResultTexture2d.Get(), &ssaoResultTexture2dSRVDesc, ssaoResultTexture2dSRV.GetAddressOf()
	));

	D3D11_RENDER_TARGET_VIEW_DESC ssaoResultTexture2dRTVDesc;
	ZeroMemory(&ssaoResultTexture2dRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	ssaoResultTexture2dRTVDesc.Format = DXGI_FORMAT_R8_UNORM;
	ssaoResultTexture2dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	ssaoResultTexture2dRTVDesc.Texture2D.MipSlice = 0;

	CHECK_DX_ERROR(_device->CreateRenderTargetView(
		ssaoResultTexture2d.Get(), &ssaoResultTexture2dRTVDesc, ssaoResultTexture2dRTV.GetAddressOf()
	));
}
}