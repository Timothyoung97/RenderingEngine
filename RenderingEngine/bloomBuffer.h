#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "window.h"
#include "dxdebug.h"

using Microsoft::WRL::ComPtr;

namespace tre {

class BloomBuffer {
public:
	ComPtr<ID3D11Texture2D> bloomTexture2D[2];

	void create(ID3D11Device* pDevice);
};
}