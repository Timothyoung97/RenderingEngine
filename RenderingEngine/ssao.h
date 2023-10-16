#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace tre {

class SSAO {

public:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	std::vector<XMFLOAT4> ssaoKernalSamples;

	ComPtr<ID3D11Texture2D> ssaoNoiseTexture2d;
	ComPtr<ID3D11ShaderResourceView> ssaoNoiseTexture2dSRV;

	ComPtr<ID3D11Texture2D> ssaoResultTexture2d;
	ComPtr<ID3D11ShaderResourceView> ssaoResultTexture2dSRV;
	ComPtr<ID3D11RenderTargetView> ssaoResultTexture2dRTV;

	ComPtr<ID3D11Texture2D> ssaoBlurredTexture2d;
	ComPtr<ID3D11ShaderResourceView> ssaoBlurredTexture2dSRV;
	ComPtr<ID3D11RenderTargetView> ssaoBlurredTexture2dRTV;

	void create(ID3D11Device* device, ID3D11DeviceContext* context);

};
}