#pragma once

#include <d3d11.h>
#include <wrl/client.h>	

using Microsoft::WRL::ComPtr;

namespace tre {

class Rasterizer {

public:

	ComPtr<ID3D11RasterizerState> pRasterizerStateFCCW;
	ComPtr<ID3D11RasterizerState> pRasterizerStateNoCull;
	D3D11_RASTERIZER_DESC rasterizerDesc;

	Rasterizer(ID3D11Device* device);

	void createRasterizer(ID3D11Device* device);
};
}