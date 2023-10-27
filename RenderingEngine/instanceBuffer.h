#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <vector>

#include "object.h"
#include "mesh.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace tre {

struct InstanceInfo {
	XMMATRIX transformation;
	XMMATRIX normalMatrix;
	XMFLOAT4 color;
	UINT isWithTexture;
	UINT hasNormMap;
};

class InstanceBuffer {

public:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	ComPtr<ID3D11Buffer> pInstanceBuffer;
	ComPtr<ID3D11ShaderResourceView> instanceBufferSRV;

	int lastBufferSize = 100; // hardcoded initial buffer size

	void createBuffer(ID3D11Device* _device, ID3D11DeviceContext* _context);

	// to be called per frame before opaque draw calls
	void updateBuffer(const std::vector<InstanceInfo>& infoQ);
};
}