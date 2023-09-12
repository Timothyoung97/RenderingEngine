#pragma once

#include <DirectXMath.h>

#include <vector>

using namespace DirectX;

struct BoundingSphere {
	XMFLOAT3 center{ .0f, .0f, .0f };
	float radius = .0f;
};

struct AABB {
	XMFLOAT3 center{ .0f, .0f, .0f };
	XMFLOAT3 halfExtent{ .0f, .0f, .0f };
};

namespace tre {

enum BoundVolumeEnum { 
	RitterBoundingSphere,
	NaiveBoundingSphere,
	AABBBoundingBox
};

class BoundingVolume {
public:
	static BoundingSphere createRitterBS(const std::vector<XMFLOAT3>& uniquePoint);
	static BoundingSphere createNaiveBS(const std::vector<XMFLOAT3>& uniquePoint);
	static AABB createAABB(const std::vector<XMFLOAT3>& uniquePoint);

	static XMMATRIX updateBoundingSphere(BoundingSphere& sphere, XMFLOAT3 scale, XMFLOAT3 rotation, XMFLOAT3 position);
	static XMMATRIX updateAABB(AABB& sphere, XMFLOAT3 scale, XMFLOAT3 rotation, XMFLOAT3 position);
};

}