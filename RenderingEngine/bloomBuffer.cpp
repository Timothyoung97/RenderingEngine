#include "bloomBuffer.h"
#include "window.h"
#include "dxdebug.h"

namespace tre {

BloomBuffer::BloomBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {
	
	D3D11_TEXTURE2D_DESC bloomTexture2DDesc;
	bloomTexture2DDesc.Width = (UINT)SCREEN_WIDTH;
	bloomTexture2DDesc.Height = (UINT)SCREEN_HEIGHT;
	bloomTexture2DDesc.MipLevels = 1u;
	bloomTexture2DDesc.ArraySize = 2u;
	bloomTexture2DDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	bloomTexture2DDesc.SampleDesc.Count = 1u;
	bloomTexture2DDesc.SampleDesc.Quality = 0u;
	bloomTexture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	bloomTexture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bloomTexture2DDesc.CPUAccessFlags = 0u;
	bloomTexture2DDesc.MiscFlags = 0u;

	CHECK_DX_ERROR(pDevice->CreateTexture2D(
		&bloomTexture2DDesc, nullptr, bloomTexture2D.GetAddressOf()
	));
}

}