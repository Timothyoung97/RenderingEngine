#pragma once

#include <d3d11.h>
#include <wrl/client.h>	

using Microsoft::WRL::ComPtr;

namespace tre {

class Rasterizer {

public:

	ComPtr<ID3D11RasterizerState> pRasterizerStateFCCW;
	ComPtr<ID3D11RasterizerState> pRasterizerStateNoCull;
	ComPtr<ID3D11RasterizerState> pRasterizerStateWireFrame;
	ComPtr<ID3D11RasterizerState> pShadowRasterizerState;

	Rasterizer(ID3D11Device* device);
};
}