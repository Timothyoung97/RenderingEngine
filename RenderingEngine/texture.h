#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>
#include <assimp/material.h>

using Microsoft::WRL::ComPtr;

namespace tre {

struct Texture {
	bool hasAlphaChannel = false;
	ComPtr<ID3D11ShaderResourceView> pShaderResView;
	aiTextureType textureType;
};

class TextureLoader {

public:
	static Texture createTexture(ID3D11Device* device, std::string filepath, aiTextureType textureType);
};
}