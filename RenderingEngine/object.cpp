#include <assert.h>

#include "object.h"
#include "utility.h"

XMFLOAT4 colors[10] = {
	{0, 0, 0, 0},
	{1, 0, 0, 1},
	{1, 1, 0, 1},
	{1, 1, 1, 1},
	{1, 0, 1, 1},
	{0, 1, 0, 1},
	{0, 0, 1, 1},
	{.5, 0, 0, 1},
	{0, .5, 0, 1},
	{0, 0, .5, 1}
};

namespace tre {

Object::Object(Mesh* pMesh, Texture* pTexture, XMFLOAT3 position, XMFLOAT3 scale, XMFLOAT3 rotation) {
	assert(pMesh != nullptr);

	if (pTexture == nullptr) {
		createObj(pMesh, nullptr, position, scale, rotation, FALSE, colors[Utility::getRandomInt(9)]);
	}
	else {
		createObj(pMesh, pTexture, position, scale, rotation, TRUE, XMFLOAT4());
	}
}

void Object::createObj(Mesh* pMesh, Texture* pTexture, XMFLOAT3 position, XMFLOAT3 scale, XMFLOAT3 rotation, bool isWithTexture, XMFLOAT4 color) {
	pObjMesh = pMesh;
	pObjTexture = pTexture;
	objPos = position;
	objScale = scale;
	objRotation = rotation;
	isObjWithTexture = isWithTexture;
	objColor = color;
}

}