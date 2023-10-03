#include "scene.h"

#include "colors.h"
#include "utility.h"

namespace tre {

Scene::Scene(ID3D11Device* device) {

	this->_objQ.reserve(1000); // hardcoded capacity

	_debugMeshes = {
		tre::CubeMesh(device), // Bounding WireMesh
		tre::SphereMesh(device, 20, 20), // Bounding WireMesh
		tre::TeapotMesh(device),
		tre::CubeMesh(device), // floor
		tre::CubeMesh(device), // testing cube
		tre::SphereMesh(device, 20, 20) // testing sphere
	};
	
	// Create testing texture
	std::string basePathStr = Utility::getBasePathStr();
	_debugTextures = {
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image.jpg"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image2.jpg"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image_a.png"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\glTF.png"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\wall.jpg"),
		tre::Texture()
	};

	_debugNormalTextures = {
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\glTF_normal.png"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\wall_normal.jpg")
	};

	_debugMaterials = {
		Material {&_debugTextures[3], &_debugNormalTextures[0]},
		Material {&_debugTextures[4], &_debugNormalTextures[1]},
		Material {nullptr, nullptr, tre::colorF(Colors::White)},
		Material {&_debugTextures[2], nullptr},
	};

	// Pt Lights
	lightResc.create(device);
	lightResc.pointLights = {
		{ XMFLOAT3(.0f, .0f, .0f), .0f, XMFLOAT3(3.0f, 3.0f, 3.0f), 100.0f, XMFLOAT3(.0f, .2f, .0f), .0f, XMFLOAT4(.1f, .1f, .1f, .1f), XMFLOAT4(.5f, .5f, .5f, .5f) },
		{ XMFLOAT3(.0f, .0f, .0f), .0f, XMFLOAT3(-3.0f, -3.0f, -3.0f), 100.0f, XMFLOAT3(.0f, .2f, .0f), .0f, XMFLOAT4(.1f, .1f, .1f, .1f), XMFLOAT4(.5f, .5f, .5f, .5f) },
		{ XMFLOAT3(.0f, .0f, .0f), .0f, XMFLOAT3(.0f, .0f, .0f), 100.0f, XMFLOAT3(.0f, .2f, .0f), .0f, XMFLOAT4(.1f, .1f, .1f, .1f), XMFLOAT4(.5f, .5f, .5f, .5f) },
		{ XMFLOAT3(.0f, .0f, .0f), .0f, XMFLOAT3(-1.0f, .0f, -1.0f), 100.0f, XMFLOAT3(.0f, .2f, .0f), .0f, XMFLOAT4(.1f, .1f, .1f, .1f), XMFLOAT4(.5f, .5f, .5f, .5f) }
	};

	for (int i = 0; i < 4; i++) {
		tre::Object newLightObj;

		newLightObj.pObjMeshes = { &_debugMeshes[1] }; // sphere
		newLightObj.pObjMeshes[0]->material = &_debugMaterials[2];
		newLightObj.objPos = lightResc.pointLights[i].pos;
		newLightObj.objScale = XMFLOAT3(.1f, .1f, .1f);
		newLightObj.objRotation = XMFLOAT3(.0f, .0f, .0f);
		newLightObj._boundingVolumeColor = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };

		_objQ.push_back(newLightObj);
		_wireframeObjQ.push_back(std::make_pair(&_objQ.back(), _objQ.back().pObjMeshes[0]));
	}
}
	
void Scene::createFloor() {

	_floor.pObjMeshes = { &_debugMeshes[3] };
	_floor.pObjMeshes[0]->material = &_debugMaterials[2];
	_floor.objPos = XMFLOAT3(.0f, .0f, .0f);
	_floor.objScale = XMFLOAT3(100.f, 0.01f, 100.f);
	_floor.objRotation = XMFLOAT3(.0f, .0f, .0f);
	_floor._boundingVolumeColor = { tre::colorF(Colors::WhiteSmoke) };
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