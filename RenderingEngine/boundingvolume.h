#pragma once

#include <DirectXMath.h>

#include <vector>

using namespace DirectX;

struct BoundingSphere {
	XMFLOAT3 center{ .0f, .0f, .0f };
	float radius = .0f;
};

struct BoundingVolume {
	XMFLOAT3 center{ .0f, .0f, .0f };
	XMFLOAT3 halfExtent{ .0f, .0f, .0f };
};

namespace tre {

enum BoundVolumeEnum { 
	RitterBoundingSphere,
	NaiveBoundingSphere,
	AABBBoundingBox
};

class RitterBS {
public:
	static BoundingSphere createRitterBS(const std::vector<XMFLOAT3>& uniquePoint);
};

class NaiveBS {
public:
	static BoundingSphere createNaiveBS(const std::vector<XMFLOAT3>& uniquePoint);
};

class AABB{
public:
	static BoundingVolume createAABB(const std::vector<XMFLOAT3>& uniquePoint);
	void update(const XMMATRIX& transformation, const XMVECTOR& translation);
};

}