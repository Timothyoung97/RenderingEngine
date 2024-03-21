#pragma once

#include <wrl/client.h>
#include <d3d11.h>

using Microsoft::WRL::ComPtr;

namespace tre {
class BlendState {
public:
	ComPtr<ID3D11BlendState> opaque;
	ComPtr<ID3D11BlendState> transparency;
	ComPtr<ID3D11BlendState> lighting;

	void create(ID3D11Device* device);
};
}