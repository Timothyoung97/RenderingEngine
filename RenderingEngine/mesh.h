#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <vector>

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 uvCoord;
};

namespace tre {

class Mesh {
public:

	ComPtr<ID3D11Buffer> pIndexBuffer;
	ComPtr<ID3D11Buffer> pVertexBuffer;
	int indexSize;
	void createVertexAndIndexBuffer(ID3D11Device* device, const vector<Vertex> &vertices, const vector<uint16_t> &indices);

};

class CubeMesh : public Mesh {
public:

	CubeMesh(ID3D11Device* device);
	void create(ID3D11Device* device);

};

class SphereMesh : public Mesh {
public:

	SphereMesh(ID3D11Device* device, int sectorC, int stackC);
	
	void create(ID3D11Device* device, int sectorC, int stackC);
	XMFLOAT3 findCoordinate(XMFLOAT3 unitVector, float radius);
};
}