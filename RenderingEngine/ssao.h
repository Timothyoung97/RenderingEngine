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
	ComPtr<ID3D11ShaderResourceView> ssaoKernelSRV;

	ComPtr<ID3D11Texture2D> ssaoNoiseTexture2D;
	ComPtr<ID3D11ShaderResourceView> ssaoKernelSRV;

	void create(ID3D11Device* device, ID3D11DeviceContext* context);

};
}