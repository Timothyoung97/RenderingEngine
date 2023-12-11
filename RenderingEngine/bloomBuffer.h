#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {

class BloomBuffer {
public:
	ComPtr<ID3D11Texture2D> bloomTexture2D;

	BloomBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};
}