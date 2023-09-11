#pragma once

#include <DirectXMath.h>

#include "mesh.h"
#include "texture.h"
#include "boundingvolume.h"

using namespace DirectX;

namespace tre {
	
struct Object {
	Mesh* pObjMesh;
	
	BoundingVolume ritterBs;
	BoundingVolume naiveBs;
	BoundingVolume aabb;
	
	Texture* pObjTexture;
	Texture* pObjNormalMap;

	XMFLOAT3 objPos;
	XMFLOAT3 objScale;
	XMFLOAT3 objRotation;

	bool isObjWithTexture;
	bool isObjWithNormalMap;

	XMFLOAT4 objColor;

	float distFromCam;
};

}