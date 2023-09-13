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

struct BoundingSphere {
	XMFLOAT3 center{ .0f, .0f, .0f };
	float radius = .0f;
	bool testBoundingSphere(BoundingSphere& other);
	bool overlapBoundingSphere(BoundingSphere& other);
};

struct AABB {
	XMFLOAT3 center{ .0f, .0f, .0f };
	XMFLOAT3 halfExtent{ .0f, .0f, .0f };
	bool testAABB(AABB& other);
	bool overlapAABB(AABB& other);
};

class BoundingVolume {
public:
	static BoundingSphere createRitterBS(const std::vector<XMFLOAT3>& uniquePoint);
	static BoundingSphere createNaiveBS(const std::vector<XMFLOAT3>& uniquePoint);
	static AABB createAABB(const std::vector<XMFLOAT3>& uniquePoint);

	static XMMATRIX updateBoundingSphere(BoundingSphere& meshSphere, BoundingSphere& objSphere, XMFLOAT3 scale, XMFLOAT3 rotation, XMFLOAT3 position);
	static XMMATRIX updateAABB(AABB& meshAABB, AABB& objAABB, XMFLOAT3 scale, XMFLOAT3 rotation, XMFLOAT3 position);
};

}