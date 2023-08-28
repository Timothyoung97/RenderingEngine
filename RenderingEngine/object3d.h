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

class Object3d {
public:

	vector<Vertex> vertices;
	vector<uint16_t> indices;
};

class Cube3d : public Object3d {
public:

	Cube3d(float unitLength);
};

class Sphere3d : public Object3d {
public:

	Sphere3d(float r, int sectorCount, int stackCount);
	XMFLOAT3 findCoordinate(XMFLOAT3 unitVector, float radius);
};
}