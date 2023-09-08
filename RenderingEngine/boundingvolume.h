#pragma once


#include "mesh.h"

namespace tre {

class BoundingSphere {
public:
	XMFLOAT3 sphereCenter{ .0f, .0f, .0f };
	float radius = .0f;
};

class RitterBS : public BoundingSphere {
public:
	RitterBS(const std::vector<XMFLOAT3>& uniquePoint);

};

class NaiveBS : public BoundingSphere {
public:
	NaiveBS(const std::vector<XMFLOAT3>& uniquePoint);
};

}