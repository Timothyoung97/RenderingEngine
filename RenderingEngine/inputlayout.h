#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "dxdebug.h"
#include "shader.h"

using Microsoft::WRL::ComPtr;

namespace tre {
class InputLayout {
public:
	D3D11_INPUT_ELEMENT_DESC layout[4] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	const UINT numOfInputElement = ARRAYSIZE(layout);

	ComPtr<ID3D11InputLayout> vertLayout;

	void create(ID3D11Device* device, VertexShader* vertShader);

};
}