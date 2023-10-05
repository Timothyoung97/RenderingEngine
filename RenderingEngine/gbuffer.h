#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {

class GBuffer {
public:
	ComPtr<ID3D11ShaderResourceView> pShaderResView;
	ComPtr<ID3D11RenderTargetView> pRenderTargetView;

	void create(ID3D11Device* device);
};
}