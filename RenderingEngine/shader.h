#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

#include <string>

#include "dxdebug.h"

using Microsoft::WRL::ComPtr;

namespace tre {
class Shader {
public:
	ComPtr<ID3DBlob> pBlob;
};

class VertexShader : public Shader {
public:
	ComPtr<ID3D11VertexShader> pShader;
	void create(std::wstring fPath, ID3D11Device* device);

};

class PixelShader : public Shader {
public:
	ComPtr<ID3D11PixelShader> pShader;
	void create(std::wstring fPath, ID3D11Device* device);
};

class ComputeShader : public Shader {
public:
	ComPtr<ID3D11ComputeShader> pShader;
	void create(std::wstring fPath, ID3D11Device* device);
};

}