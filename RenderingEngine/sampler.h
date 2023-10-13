#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {

class Sampler {

public:
	ComPtr<ID3D11SamplerState> pSamplerStateLinear;
	ComPtr<ID3D11SamplerState> pSamplerStateMipPtWhiteBorder;
	ComPtr<ID3D11SamplerState> pSamplerStateMipPtWrap;

	void create(ID3D11Device* device);
};
}