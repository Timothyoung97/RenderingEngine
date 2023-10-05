#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {

enum GBUFFER_TYPE {
	NORMAL_T,
	ALBEDO_T
};

class GBuffer {

public:
	ComPtr<ID3D11Texture2D> pGBufferTextureAlbedo;
	ComPtr<ID3D11ShaderResourceView> pShaderResViewDeferredAlbedo;
	ComPtr<ID3D11RenderTargetView> pRenderTargetViewDeferredAlbedo;

	ComPtr<ID3D11Texture2D> pGBufferTextureNormal;
	ComPtr<ID3D11ShaderResourceView> pShaderResViewDeferredNormal;
	ComPtr<ID3D11RenderTargetView> pRenderTargetViewDeferredNormal;

	void create(ID3D11Device* device);
};
}