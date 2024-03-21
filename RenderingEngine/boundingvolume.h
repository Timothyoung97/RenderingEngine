#pragma once

#include <vector>
#include <string>

#include "maths.h"
#include "utility.h"
#include "Global_Config.h"

using namespace DirectX;

namespace tre {

enum BoundVolumeEnum { 
	RitterBoundingSphere,
	NaiveBoundingSphere,
	AABBBoundingBox
};

inline const char* ToString(BoundVolumeEnum bv)
{
	switch (bv)
	{
	case RitterBoundingSphere:	return "Ritter Sphere";
	case NaiveBoundingSphere:	return "Naive Sphere";
	case AABBBoundingBox:		return "AABB";
	default:					return "[Unknown Bounding Mode]";
	}
}

struct BoundingSphere {
	XMFLOAT3 center{ .0f, .0f, .0f };
	float radius = .0f;

	bool testBoundingSphere(BoundingSphere& other);
	bool overlapBoundingSphere(BoundingSphere& other);

	bool isForwardPlane(Plane& plane); // check if a bounding sphere is completely forward of the plane
	bool isInFrustum(Frustum& camFrustum); // check if a bounding sphere is completely inside the frustum

	bool isOnPlane(Plane& plane); // check if a bounding sphere is overlapping the plane
	bool isOverlapFrustum(Frustum& camFrustum); // check if a bounding sphere is intersecting with the frustum

};

struct AABB {
	XMFLOAT3 center{ .0f, .0f, .0f };
	XMFLOAT3 halfExtent{ .0f, .0f, .0f };

	bool testAABB(AABB& other);
	bool overlapAABB(AABB& other);

	bool isForwardPlane(Plane& plane); // check if an AABB  is completely forward of the plane
	bool isInFrustum(Frustum& camFrustum); // check if an AABB is completely inside the frustum

	bool isOnPlane(Plane& plane); // check if an AABB is overlapping the plane
	bool isOverlapFrustum(Frustum& camFrustum); // check if an AABB is intersecting with the frustum
};

class BoundingVolume {
public:
	static BoundingSphere createRitterBS(const std::vector<XMFLOAT3>& uniquePoint);
	static BoundingSphere createNaiveBS(const std::vector<XMFLOAT3>& uniquePoint);
	static AABB createAABB(const std::vector<XMFLOAT3>& uniquePoint);

	static XMMATRIX updateBoundingSphere(BoundingSphere& meshSphere, BoundingSphere& objSphere, XMMATRIX transformation);
	static XMMATRIX updateAABB(AABB& meshAABB, AABB& objAABB, XMMATRIX transformation);
};

}