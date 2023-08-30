#pragma once

#include "mesh.h"

namespace tre {

class Object {
public:
	Mesh* objectMesh;
	// make a pointer to texture

	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT3 rotation;

};

}