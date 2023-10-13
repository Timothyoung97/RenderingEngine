#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {

class SSAO {
	
	// todo: add noise texture
	// todo: add hemisphere

public:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	ComPtr<ID3D11Buffer> ssaoKernelBuffer;
	ComPtr<ID3D11ShaderResourceView> ssaoKernelBufferSRV;

	ComPtr<ID3D11Texture2D> ssaoNoiseTexture2d;
	ComPtr<ID3D11ShaderResourceView> ssaoNoiseTexture2dSRV;

	ComPtr<ID3D11Texture2D> ssaoResultTexture2d;
	ComPtr<ID3D11ShaderResourceView> ssaoResultTexture2dSRV;
	ComPtr<ID3D11RenderTargetView> ssaoResultTexture2dRTV;

	void create(ID3D11Device* device, ID3D11DeviceContext* context);

};
}