#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "dxdebug.h"

using Microsoft::WRL::ComPtr;

namespace tre {

class DepthBuffer {
public:
	ComPtr<ID3D11Texture2D> pDepthStencilTexture;

	ComPtr<ID3D11Texture2D> pDepthStencilReadOnlyTexture;

	ComPtr<ID3D11Texture2D> pShadowMapTexture;

	ComPtr<ID3D11DepthStencilState> pDSStateWithDepthTWriteEnabled;
	ComPtr<ID3D11DepthStencilState> pDSStateWithDepthTWriteDisabled;
	ComPtr<ID3D11DepthStencilState> pDSStateWithoutDepthT;

	void create(ID3D11Device* device, int screenW, int screenH);
};

}