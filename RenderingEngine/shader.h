#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>

using Microsoft::WRL::ComPtr;

namespace tre {
class Shader {
public:
	ComPtr<ID3DBlob> pBlob;
};

class VertexShader : public Shader {
public:
	ComPtr<ID3D11VertexShader> pShader;
	VertexShader(std::wstring fPath, ID3D11Device* device);

};

class PixelShader : public Shader {
public:
	ComPtr<ID3D11PixelShader> pShader;
	PixelShader(std::wstring fPath, ID3D11Device* device);
};

}