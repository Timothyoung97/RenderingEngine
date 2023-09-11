#pragma once

#include <DirectXMath.h>

#include <vector>

using namespace DirectX;

namespace tre {

enum BoundVolumeEnum { 
	RitterBoundingSphere,
	NaiveBoundingSphere,
	AABBBoundingBox
};

class BoundingVolume {
public:
	XMFLOAT3 center{ .0f, .0f, .0f };
	XMFLOAT3 halfExtent{ .0f, .0f, .0f };
	float radius = .0f;
};

class RitterBS : public BoundingVolume {
public:
	RitterBS(const std::vector<XMFLOAT3>& uniquePoint);
};

class NaiveBS : public BoundingVolume {
public:
	NaiveBS(const std::vector<XMFLOAT3>& uniquePoint);
};

class AABB : public BoundingVolume {
public:
	AABB(const std::vector<XMFLOAT3>& uniquePoint);
	void update(const XMMATRIX& transformation, const XMVECTOR& translation);
};

}