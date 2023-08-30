#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {

class Sampler {

public:
	ComPtr<ID3D11SamplerState> pSamplerState;

	Sampler(ID3D11Device* device);
};
}