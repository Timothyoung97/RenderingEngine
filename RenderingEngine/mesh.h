#pragma once

#include <DirectXMath.h>
#include <vector>

using namespace DirectX;
using namespace std;

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 uvCoord;
};

namespace tre {

class Mesh {
public:

	vector<Vertex> vertices;
	vector<uint16_t> indices;
};

class CubeMesh : public Mesh {
public:

	CubeMesh(float unitLength);
	CubeMesh(XMFLOAT3 origin, float unitLength);
	void create(XMFLOAT3 origin, float unitLength);
};

class SphereMesh : public Mesh {
public:

	SphereMesh(float r, int sectorC, int stackC);
	SphereMesh(XMFLOAT3 origin, float r, int sectorC, int stackC);
	
	void create(XMFLOAT3 origin, float r, int sectorC, int stackC);
	XMFLOAT3 findCoordinate(XMFLOAT3 unitVector, float radius);
};
}