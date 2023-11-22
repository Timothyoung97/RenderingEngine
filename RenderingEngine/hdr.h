#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "shader.h"

using Microsoft::WRL::ComPtr;

namespace tre {

class HdrBuffer {

public:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	ComPtr<ID3D11Texture2D> pHdrBufferTexture;
	ComPtr<ID3D11ShaderResourceView> pShaderResViewHdrTexture;
	ComPtr<ID3D11RenderTargetView> pRenderTargetViewHdrTexture;

	ComPtr<ID3D11Buffer> pLuminHistogram; // size of 256
	ComPtr<ID3D11UnorderedAccessView> pLuminHistogramUAV;

	ComPtr<ID3D11Buffer> pLuminAvg;	// size of 1
	ComPtr<ID3D11ShaderResourceView> pLuminAvgSRV;
	ComPtr<ID3D11UnorderedAccessView> pLuminAvgUAV;

	void create(ID3D11Device* device, ID3D11DeviceContext* context);

};
}
