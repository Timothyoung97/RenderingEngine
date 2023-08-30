#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include <dxdebug.h>

using namespace DirectX;
using namespace std;

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 uvCoord;
};

namespace tre {

class Mesh {
public:

	ID3D11Buffer* pIndexBuffer;
	ID3D11Buffer* pVertexBuffer;
	int indexSize;

	void createVertexAndIndexBuffer(ID3D11Device* device, vector<Vertex> vertices, vector<uint16_t> indices);
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