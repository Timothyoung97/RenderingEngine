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

class Cube3d
{
public:
	vector<Vertex> vertices;
	vector<uint16_t> indices;

	Cube3d(float unitLength);
};

class Sphere3d
{
public:
	vector<Vertex> vertices;
	vector<uint16_t> indices;

	Sphere3d(float r, int sectorCount, int stackCount);

	XMFLOAT3 findCoordinate(XMFLOAT3 unitVector, float radius);
	XMVECTOR convertToVector(float vectorComponentValue, int idx);
};
}