#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <vector>

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct constBufferShaderResc {
	XMMATRIX matrix;
};

namespace tre {

class ConstantBuffer {
public:
	D3D11_BUFFER_DESC constantBufferDesc;
	D3D11_SUBRESOURCE_DATA csd = {};
	ComPtr<ID3D11Buffer> pConstBuffer;

	constBufferShaderResc constBufferCamResc;
	constBufferShaderResc constBufferModelResc;

	vector<constBufferShaderResc> constBufferShaderRescList;

	ConstantBuffer();
};

}