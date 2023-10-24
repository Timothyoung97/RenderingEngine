#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {

class HdrBuffer {

public:

	ComPtr<ID3D11Texture2D> pHdrBufferTexture;
	ComPtr<ID3D11ShaderResourceView> pShaderResViewHdrTexture;
	ComPtr<ID3D11RenderTargetView> pRenderTargetViewHdrTexture;

	void create(ID3D11Device* device);
};
}
