#pragma once

#include <DirectXMath.h>

#include "mesh.h"
#include "texture.h"
#include "boundingvolume.h"

#include <vector>

using namespace DirectX;

namespace tre {
	
struct Object {

	Object* parent = nullptr;
	std::vector<Object> children;

	Mesh* pObjMesh = nullptr;
	
	BoundingSphere ritterBs;
	BoundingSphere naiveBs;
	AABB aabb;

	XMFLOAT3 objPos;
	XMFLOAT3 objScale;
	XMFLOAT3 objRotation;

	XMMATRIX _transformationFinal;

	XMFLOAT4 _boundingVolumeColor;

	float distFromCam;

	XMMATRIX makeLocalToWorldMatrix();
};

}