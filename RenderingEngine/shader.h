#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>

using Microsoft::WRL::ComPtr;

namespace tre {
class Shader {

public:
	ComPtr<ID3DBlob> pVSBlob;
	ComPtr<ID3DBlob> pPSBlob;
	ComPtr<ID3D11VertexShader> pVS;
	ComPtr<ID3D11PixelShader> pPS;

	Shader(std::wstring vsFpath, std::wstring psFpath, ID3D11Device* device);
};
}