#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>

using Microsoft::WRL::ComPtr;

namespace tre {

struct Texture {
	bool hasAlphaChannel = false;
	ComPtr<ID3D11Texture2D> pTextureResource;
};

class TextureLoader {

public:
	static Texture createTexture(ID3D11Device* device, std::string filepath);
};
}