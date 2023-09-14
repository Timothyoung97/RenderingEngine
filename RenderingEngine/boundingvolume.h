#pragma once

#include <vector>

#include "maths.h"

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
	bool isForwardPlane(Plane& plane);
	bool isInFrustum(Frustum& camFrustum);
	bool isOnPlane(Plane& plane);
	bool isOverlapFrustum(Frustum& camFrustum);

};

struct AABB {
	XMFLOAT3 center{ .0f, .0f, .0f };
	XMFLOAT3 halfExtent{ .0f, .0f, .0f };
	bool testAABB(AABB& other);
	bool overlapAABB(AABB& other);
	bool isOnOrForwardPlane(Plane& plane);
	bool isInFrustum(Frustum& camFrustum);
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