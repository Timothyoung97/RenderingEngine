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
	XMINT2 pad;
};

struct InstanceBatchInfo {
	int batchStartIdx;
	int quantity;
	int isWithTexture;
	int hasNormMap;
	Mesh* pBatchMesh;
	Texture* pBatchTexture;
	Texture* pBatchNormalMap;
};

class InstanceBuffer {

public:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	ComPtr<ID3D11Buffer> pInstanceBuffer;
	ComPtr<ID3D11ShaderResourceView> pInstanceBufferSRV;

	int lastBufferSize = 100; // hardcoded initial buffer size

	std::vector<InstanceBatchInfo> instanceBatchQueue;

	void createBuffer(ID3D11Device* _device, ID3D11DeviceContext* _context);

	// to be called per frame before opaque draw calls
	void updateBuffer(const std::vector<std::pair<Object*, Mesh*>>& objQ);

	/// <summary>
	/// Use for batching a single instance with the same mesh but no texture/normal, typically used in wireframe draw where all meshes are the same
	/// </summary>
	/// <param name="objQ"></param>
	/// <param name="specifiedMesh"></param>
	void updateBuffer(const std::vector<std::pair<Object*, Mesh*>>& objQ, Mesh* specifiedMesh);

	InstanceInfo createInstanceInfo(XMMATRIX transformationLocal, XMFLOAT4 color, UINT isWithTexture, UINT hasNormalMap);
};
}