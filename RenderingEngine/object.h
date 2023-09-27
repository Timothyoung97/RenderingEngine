#pragma once

#include <DirectXMath.h>

#include "mesh.h"
#include "texture.h"
#include "boundingvolume.h"

using namespace DirectX;

namespace tre {
	
struct Object {
	Mesh* pObjMesh;
	
	BoundingSphere ritterBs;
	BoundingSphere naiveBs;
	AABB aabb;
	
	XMFLOAT3 objPos;
	XMFLOAT3 objScale;
	XMFLOAT3 objRotation;

	XMFLOAT4 _boundingVolumeColor;

	float distFromCam;
};

}