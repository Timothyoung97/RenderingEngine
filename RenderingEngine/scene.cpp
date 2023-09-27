#include "scene.h"

#include "utility.h"

namespace tre {

Scene::Scene(ID3D11Device* device) {

	_debugMeshes = {
		tre::CubeMesh(device),
		tre::SphereMesh(device, 20, 20),
		tre::TeapotMesh(device)
	};
	
	// Create testing texture
	std::string basePathStr = Utility::getBasePathStr();
	_debugTextures = {
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image.jpg"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image2.jpg"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image_a.png"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\glTF.png"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\wall.jpg")
	};

	_debugNormalTextures = {
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\glTF_normal.png"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\wall_normal.jpg")
	};

}
	
void Scene::createFloor() {
	_floor.pObjMesh = &_debugMeshes[0];
	_floor.objPos = XMFLOAT3(.0f, .0f, .0f);
	_floor.objScale = XMFLOAT3(100.f, 0.01f, 100.f);
	_floor.objRotation = XMFLOAT3(.0f, .0f, .0f);
	_floor.pObjTexture = &_debugTextures[0];
	_floor.pObjNormalMap = nullptr;
	_floor.isObjWithTexture = 0;
	_floor.isObjWithNormalMap = 0;
	_floor.objColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
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