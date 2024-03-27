#include "gbuffer.h"

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
	gbufferDesc.CPUAccessFlags = 0u;
	gbufferDesc.MiscFlags = 0;

	CHECK_DX_ERROR(device->CreateTexture2D(
		&gbufferDesc, nullptr, pGBufferTextureAlbedo.GetAddressOf()
	)); 

	CHECK_DX_ERROR(device->CreateTexture2D(
		&gbufferDesc, nullptr, pGBufferTextureNormal.GetAddressOf()
	));
}
}