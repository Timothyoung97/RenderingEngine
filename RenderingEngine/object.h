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

	std::vector<Mesh*> pObjMeshes;
	
	std::vector<BoundingSphere> ritterBs;
	std::vector<BoundingSphere> naiveBs;
	std::vector<AABB> aabb;
	std::vector<XMFLOAT4> _boundingVolumeColor;
	std::vector<XMMATRIX> _boundingVolumeTransformation;

	XMFLOAT3 objPos;
	XMFLOAT3 objScale;
	XMFLOAT3 objRotation;

	XMMATRIX _transformationFinal;
	aiMatrix4x4 _transformationAssimp;

	float distFromCam;
};

class ObjectUtility {
public:
	static void updateBoundingVolumeTransformation(Object& obj, BoundVolumeEnum typeOfBound);
	static XMMATRIX makeLocalToWorldMatrix(Object& obj);
	static bool isMeshWithinView(Object& obj, int meshIdx, Frustum& frustum, BoundVolumeEnum typeOfBound, bool toChangeColor);

};

}