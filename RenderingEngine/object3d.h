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

//class Sphere3d
//{
//public:
//	vector<Vertex> sphere3dVertices;
//	vector<uint16_t> sphere3dIndices;
//
//	Sphere3d();
//};
}