#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <assimp/scene.h>

#include <vector>

#include "Global_Config.h"
#include "material.h"
#include "boundingvolume.h"
#include "dxdebug.h"
#include "utility.h"
#include "maths.h"


using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 uvCoord;
};

namespace tre {

class Mesh {
public:

	ComPtr<ID3D11Buffer> pIndexBuffer;
	ComPtr<ID3D11Buffer> pVertexBuffer;

	Material* pMaterial;

	BoundingSphere ritterSphere;
	BoundingSphere naiveSphere;
	AABB aabb;

	int indexSize;
	void createVertexAndIndexBuffer(ID3D11Device* device, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

};

class CustomMesh : public Mesh {
public:
	CustomMesh(ID3D11Device* device, aiMesh* mesh);
};

class CubeMesh : public Mesh {
public:
	CubeMesh(ID3D11Device* device);
	void create(ID3D11Device* device);

};

class SphereMesh : public Mesh {
public:
	SphereMesh(ID3D11Device* device, int sectorC, int stackC);
	SphereMesh(ID3D11Device* device, int sectorC, int stackC, float r);

	void create(ID3D11Device* device, int sectorC, int stackC, float r);
	XMFLOAT3 findCoordinate(XMFLOAT3 unitVector, float radius);
};

class TeapotMesh : public Mesh {
public:

	TeapotMesh(ID3D11Device* device);
};

}