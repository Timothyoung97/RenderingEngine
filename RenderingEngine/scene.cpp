#include "scene.h"

#include "mesh.h"
#include "texture.h"

namespace tre {
	
void Scene::createFloor(Mesh* mesh, Texture* texture) {
	floor.pObjMesh = mesh;
	floor.objPos = XMFLOAT3(.0f, .0f, .0f);
	floor.objScale = XMFLOAT3(100.f, 100.f, 100.f);
	floor.objRotation = XMFLOAT3(.0f, .0f, .0f);
	floor.pObjTexture = texture;
	floor.pObjNormalMap = nullptr;
	floor.isObjWithTexture = 1;
	floor.isObjWithNormalMap = 0;
	floor.objColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

}