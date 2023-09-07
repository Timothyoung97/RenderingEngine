#pragma once


#include "mesh.h"

namespace tre {

class RitterBS {
public:

XMFLOAT3 sphereCenter{ .0f, .0f, .0f };
float radius = .0f;
	
RitterBS(const std::vector<XMFLOAT3>& uniquePoint);

};
}