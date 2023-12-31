#include <DirectXMath.h>

#include <vector>

#include "ssao.h"
#include "utility.h"
#include "dxdebug.h"
#include "window.h"

using namespace DirectX;

namespace tre {

void SSAO::create(ID3D11Device* device) {
	_device = device;

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
}
}