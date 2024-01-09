#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "shader.h"

using Microsoft::WRL::ComPtr;

namespace tre {

class HdrBuffer {

public:
	ID3D11Device* _device;

	ComPtr<ID3D11Texture2D> pHdrBufferTexture;

	ComPtr<ID3D11Buffer> pLuminHistogram; // size of 256

	ComPtr<ID3D11Buffer> pLuminAvg;	// size of 1

	void create(ID3D11Device* device);

};
}
