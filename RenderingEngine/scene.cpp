#include "scene.h"

#include "utility.h"

namespace tre {

Scene::Scene(ID3D11Device* device) {

	meshes = {
		tre::CubeMesh(device),
		tre::SphereMesh(device, 20, 20),
		tre::TeapotMesh(device)
	};
	
	// Create texture
	std::string basePathStr = Utility::getBasePathStr();
	textures = {
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image.jpg", aiTextureType_DIFFUSE),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image2.jpg", aiTextureType_DIFFUSE),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image_a.png", aiTextureType_DIFFUSE),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\glTF.png", aiTextureType_DIFFUSE),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\wall.jpg", aiTextureType_DIFFUSE)
	};

	normalTextures = {
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\glTF_normal.png", aiTextureType_NORMALS),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\wall_normal.jpg", aiTextureType_NORMALS)
	};

}
	
void Scene::createFloor() {
	floor.pObjMesh = &meshes[0];
	floor.objPos = XMFLOAT3(.0f, .0f, .0f);
	floor.objScale = XMFLOAT3(100.f, 0.01f, 100.f);
	floor.objRotation = XMFLOAT3(.0f, .0f, .0f);
	floor.pObjTexture = &textures[0];
	floor.pObjNormalMap = nullptr;
	floor.isObjWithTexture = 0;
	floor.isObjWithNormalMap = 0;
	floor.objColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

void Scene::updateDirLight() {

	XMFLOAT3 dirF = tre::Maths::getRotatePosition(XMFLOAT3(.0f, .0f, .0f), dirlightPitch, dirlightYaw, 1.f);
	XMVECTOR dirV = XMLoadFloat3(&dirF);

	XMStoreFloat3(&dirF, XMVector3Normalize(dirV));

	dirlight = {
		dirF, .0f, XMFLOAT4(.5f, .5f, .5f, 1.0f), XMFLOAT4(.5f, .5f, .5f, .5f)
	};
}


}