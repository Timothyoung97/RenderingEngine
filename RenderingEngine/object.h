#pragma once

#include <DirectXMath.h>

#include "mesh.h"
#include "texture.h"

using namespace DirectX;

namespace tre {
	
class Object {
public:
	Mesh* pObjMesh;
	Texture* pObjTexture;

	XMFLOAT3 objPos;
	XMFLOAT3 objScale;
	XMFLOAT3 objRotation;

	bool isObjWithTexture;
	XMFLOAT4 objColor;

	float distFromCam;

	void createObj(Mesh* pMesh, Texture* pTexture, XMFLOAT3 position, XMFLOAT3 scale, XMFLOAT3 rotation, bool isWithTexture, XMFLOAT4 color);

};

}