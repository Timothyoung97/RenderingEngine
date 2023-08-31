#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>

using Microsoft::WRL::ComPtr;

namespace tre {

class Texture {

public:
	bool hasAlphaChannel = FALSE;

	ComPtr<ID3D11ShaderResourceView> pShaderResView;

	Texture(ID3D11Device* device, std::string filepath);

	void createTexture(ID3D11Device* device, std::string filepath);

};
}