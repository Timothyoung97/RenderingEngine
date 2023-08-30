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

	CubeMesh();
	void create();
};

class SphereMesh : public Mesh {
public:

	SphereMesh(int sectorC, int stackC);
	
	void create(int sectorC, int stackC);
	XMFLOAT3 findCoordinate(XMFLOAT3 unitVector, float radius);
};
}