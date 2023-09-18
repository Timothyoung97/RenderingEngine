#include "scene.h"

#include "mesh.h"
#include "texture.h"
#include "utility.h"

namespace tre {

Scene::Scene(ID3D11Device* device) {

	meshes = {
		tre::CubeMesh(device),
		tre::SphereMesh(device, 20, 20),
		tre::TeapotMesh(device),
		tre::FloorMesh(device)
	};
	
	// Create texture
	std::string basePathStr = Utility::getBasePathStr();
	textures = {
		tre::Texture(device, basePathStr + "textures\\UV_image.jpg"),
		tre::Texture(device, basePathStr + "textures\\UV_image2.jpg"),
		tre::Texture(device, basePathStr + "textures\\UV_image_a.png"),
		tre::Texture(device, basePathStr + "textures\\glTF.png"),
		tre::Texture(device, basePathStr + "textures\\wall.jpg")
	};

	normalTextures = {
		tre::Texture(device, basePathStr + "textures\\glTF_normal.png"),
		tre::Texture(device, basePathStr + "textures\\wall_normal.jpg")
	};

}
	
void Scene::createFloor() {
	floor.pObjMesh = &meshes[3];
	floor.objPos = XMFLOAT3(.0f, .0f, .0f);
	floor.objScale = XMFLOAT3(100.f, 100.f, 100.f);
	floor.objRotation = XMFLOAT3(.0f, .0f, .0f);
	floor.pObjTexture = &textures[0];
	floor.pObjNormalMap = nullptr;
	floor.isObjWithTexture = 1;
	floor.isObjWithNormalMap = 0;
	floor.objColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

}