#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>

using namespace std;
using Microsoft::WRL::ComPtr;

namespace tre {

class Texture {
public:
	ComPtr<ID3D11ShaderResourceView> pShaderResView;
	ComPtr<ID3D11SamplerState> pSamplerState;

	Texture(ID3D11Device* device, string filepath);

	void createTexture(ID3D11Device* device, string filepath);

};
}