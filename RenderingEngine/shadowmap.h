#pragma once

#include <wrl/client.h>
#include <d3d11.h>

using Microsoft::WRL::ComPtr;

namespace tre {
class ShadowMap {
public:
	
	ComPtr<ID3D11Texture2D> shadowMap;
	ComPtr<ID3D11DepthStencilView> shadowDepthStencilView;
	ComPtr<ID3D11ShaderResourceView> shadowShaderRescView;
	ComPtr<ID3D11SamplerState> shadowSamplerState;

	
	ShadowMap(ID3D11Device* device);

};
}