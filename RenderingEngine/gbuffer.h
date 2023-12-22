#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {

enum GBUFFER_TYPE {
	NORMAL_T,
	ALBEDO_T
};

class GBuffer {

public:
	
	ComPtr<ID3D11Texture2D> pGBufferTextureAlbedo;

	ComPtr<ID3D11Texture2D> pGBufferTextureNormal;

	void create(ID3D11Device* device);
};
}