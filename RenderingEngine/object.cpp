#include <assert.h>

#include "object.h"
#include "utility.h"


namespace tre {

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