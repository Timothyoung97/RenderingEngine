#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "dxdebug.h"

using Microsoft::WRL::ComPtr;

namespace tre {

class Sampler {

public:
	ComPtr<ID3D11SamplerState> pSamplerStateMinMagMipLinearWrap;
	ComPtr<ID3D11SamplerState> pSamplerStateMinMagMipLinearGreaterEqualBorder;
	ComPtr<ID3D11SamplerState> pSamplerStateMinMagMipPtWrap;
	ComPtr<ID3D11SamplerState> pSamplerStateMinMagMipPtClamp;
	ComPtr<ID3D11SamplerState> pSamplerStateMinMagMipLinearClamp;

	void create(ID3D11Device* device);
};
}