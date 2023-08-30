#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <vector>

#include "utility.h"

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct constBufferShaderResc {
	XMMATRIX matrix;
};

namespace tre {

class ConstantBufferManager {
public:
	D3D11_BUFFER_DESC constantBufferDesc;
	D3D11_SUBRESOURCE_DATA csd = {};
	ComPtr<ID3D11Buffer> pConstBuffer;

	constBufferShaderResc constBufferCamResc;

	vector<constBufferShaderResc> constBufferShaderRescList;
	
	ConstantBufferManager(XMMATRIX camView, XMMATRIX camProjection);

	void addNewConstBufferResc(
		float offsetX, float offsetY, float offsetZ, 
		float scaleX, float scaleY, float scaleZ, 
		float localYaw, float localPitch, float localRoll,
		XMMATRIX camView, XMMATRIX camProjection);

	void addRandomConstBufferResc(XMMATRIX camView, XMMATRIX camProjection);
};
}