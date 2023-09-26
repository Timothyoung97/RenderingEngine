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

	// Shadow Buffer Quad Coord
	tagRECT rectArr[4] = {
		{ 0, 0, 2048, 2048 }, // top left
		{ 2048, 0, 4096, 2048 }, // top right
		{ 0, 2048, 2048, 4096 }, // bottom left
		{ 2048, 2048, 4096, 4096 }  // bottom right
	};

	Rasterizer(ID3D11Device* device);
};
}