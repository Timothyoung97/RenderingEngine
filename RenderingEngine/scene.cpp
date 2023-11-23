#include "scene.h"

#include <format>

#include "microprofile.h"
#include "colors.h"
#include "utility.h"

namespace tre {

Scene::Scene(ID3D11Device* device, ID3D11DeviceContext* context) {

	this->_objQ.reserve(1000000); // hardcoded capacity

	_debugMeshes = {
		tre::CubeMesh(device), // Bounding WireMesh
		tre::SphereMesh(device, 20, 20), // Bounding WireMesh
		tre::TeapotMesh(device),
		tre::CubeMesh(device), // floor
		tre::CubeMesh(device), // transparent cube
		tre::CubeMesh(device), // testing cube
		tre::SphereMesh(device, 6, 6), // testing sphere
		tre::CubeMesh(device), // testing cube
		tre::SphereMesh(device, 6, 6), // testing sphere

	};
	
	// Create testing texture
	std::string basePathStr = Utility::getBasePathStr();
	_debugTextures = {
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image.jpg"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image2.jpg"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\UV_image_a.png"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\glTF.png"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\wall.jpg"),
	};

	_debugNormalTextures = {
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\glTF_normal.png"),
		tre::TextureLoader::createTexture(device, basePathStr + "textures\\wall_normal.jpg")
	};

	_debugMaterials = {
		Material {&_debugTextures[3], &_debugNormalTextures[0]},
		Material {&_debugTextures[4], &_debugNormalTextures[1]},
		Material {nullptr, nullptr, tre::colorF(Colors::GreenYellow)},
		Material {&_debugTextures[0], nullptr},
		Material {&_debugTextures[2], nullptr},
		Material {nullptr, nullptr, tre::colorF(Colors::LightBlue)},
	};

	// Pt Lights
	lightResc.create(device, context);
	
	// Debug 
	//lightResc.addPointLight(XMFLOAT3(3.0f, 3.0f, 3.0f),    XMFLOAT3(1.f, .14f, .07f),  XMFLOAT4(.0f, 10.f, 10.f, .5f), XMFLOAT2(.0f, .0f));
	//lightResc.addPointLight(XMFLOAT3(-3.0f, -3.0f, -3.0f), XMFLOAT3(1.f, .35f, .44f), XMFLOAT4(.0f, 1.f, .0f, .5f), XMFLOAT2(.0f, .0f));
	//lightResc.addPointLight(XMFLOAT3(1.f, 1.f, 1.0f),	   XMFLOAT3(1.f, .22f, .2f),  XMFLOAT4(.0f, .0f, 1.f, .5f), XMFLOAT2(.0f, .0f));
	//lightResc.addPointLight(XMFLOAT3(-1.0f, .0f, -1.0f),   XMFLOAT3(1.f, .22f, .27f), XMFLOAT4(1.f, 1.f, .0f, .5f), XMFLOAT2(.0f, .0f));

	createFloor();
	updateDirLight();
}
	
void Scene::createFloor() {
	_floor.pObjMeshes = { &_debugMeshes[3] };
	_floor.pObjMeshes[0]->pMaterial = &_debugMaterials[5];
	_floor.objPos = XMFLOAT3(.0f, -1.f, .0f);
	_floor.objScale = XMFLOAT3(100.f, 0.01f, 100.f);
	_floor.objRotation = XMFLOAT3(.0f, .0f, .0f);
	_floor.aabb = { _floor.pObjMeshes[0]->aabb };
	_floor.ritterBs = { _floor.pObjMeshes[0]->ritterSphere };
	_floor.naiveBs = { _floor.pObjMeshes[0]->naiveSphere };
	_floor._boundingVolumeColor = { tre::colorF(Colors::WhiteSmoke) };
	_floor._transformationFinal = tre::Maths::createTransformationMatrix(_floor.objScale, _floor.objRotation, _floor.objPos);
	_pObjQ.push_back(&_floor);
}

void Scene::updateDirLight() {

	XMFLOAT3 dirF = tre::Maths::getRotatePosition(XMFLOAT3(.0f, .0f, .0f), dirlightPitch, dirlightYaw, 1.f);
	XMVECTOR dirV = XMLoadFloat3(&dirF);

	XMStoreFloat3(&dirF, XMVector3Normalize(dirV));

	dirlight = {
		dirF, .0f, XMFLOAT4(.5f, .5f, .5f, 1.0f), XMFLOAT4(dirLightDiffuse, dirLightDiffuse, dirLightDiffuse, .0f)
	};
}

void Scene::updateBoundingVolume(BoundVolumeEnum typeOfBound) {
	const char* name = ToString(typeOfBound);
	MICROPROFILE_SCOPE_CSTR(name);

	for (int i = 0; i < _pObjQ.size(); i++) {
		Object* pObj = _pObjQ[i];
		for (int j = 0; j < pObj->pObjMeshes.size(); j++) {
			switch (typeOfBound) {
			case tre::AABBBoundingBox:
				pObj->_boundingVolumeTransformation = tre::BoundingVolume::updateAABB(pObj->pObjMeshes[j]->aabb, pObj->aabb[j], pObj->_transformationFinal);
				break;
			case tre::RitterBoundingSphere:
				pObj->_boundingVolumeTransformation = tre::BoundingVolume::updateBoundingSphere(pObj->pObjMeshes[j]->ritterSphere, pObj->ritterBs[j], pObj->_transformationFinal);
				break;
			case tre::NaiveBoundingSphere:
				pObj->_boundingVolumeTransformation = tre::BoundingVolume::updateBoundingSphere(pObj->pObjMeshes[j]->naiveSphere, pObj->naiveBs[j], pObj->_transformationFinal);
				break;
			}
		}
	}
}

void Scene::updateCulledOpaqueQ() {
	std::sort(_culledOpaqueObjQ.begin(), _culledOpaqueObjQ.end(),
		[](const std::pair<tre::Object*, tre::Mesh*> obj1, const std::pair<tre::Object*, tre::Mesh*> obj2) {
			if (obj1.second < obj2.second) {
				return true;
			}
			else if (obj1.second > obj2.second) {
				return false;
			}
			else {
				return obj1.second->pMaterial->objTexture < obj2.second->pMaterial->objTexture;
			}
		}
	);
}

void Scene::updateCulledTransparentQ(Camera& cam) {
	if (_toRecalDistFromCam) {
		for (int i = 0; i < _culledTransparentObjQ.size(); i++) {
			Object* pObj = _culledTransparentObjQ[i].first;
			// find its distance from cam
			pObj->distFromCam = tre::Maths::distBetweentObjToCam(pObj->objPos, cam.camPositionV);
		}
		
		_toSortTransparentQ = true;
		_toRecalDistFromCam = false;
	}

	// sort the vector -> object with greater dist from cam is at the front of the Q
	if (_toSortTransparentQ) {
		std::sort(_culledTransparentObjQ.begin(), _culledTransparentObjQ.end(), [](const std::pair<tre::Object*, tre::Mesh*> obj1, const std::pair<tre::Object*, tre::Mesh*> obj2) { return obj1.first->distFromCam > obj2.first->distFromCam; });
		_toSortTransparentQ = false;
	}
}

void Scene::cullObject(Frustum& frustum, BoundVolumeEnum typeOfBound) {

	MICROPROFILE_SCOPE_CSTR("cullObject");

	// clear render queue
	_culledOpaqueObjQ.clear();
	_culledTransparentObjQ.clear();

	for (int i = 0; i < _pObjQ.size(); i++) {
		Object* pObj = _pObjQ[i];
		for (int j = 0; j < pObj->pObjMeshes.size(); j++) {
			Mesh* pMesh = pObj->pObjMeshes[j];

			int isTransparent = 0;
			if ((pMesh->pMaterial->objTexture != nullptr && pMesh->pMaterial->objTexture->hasAlphaChannel)
				|| (pMesh->pMaterial->objTexture == nullptr && pMesh->pMaterial->baseColor.w < 1.0f)) {

				isTransparent = 1;
			}

			int addToQ = 0;
			switch (typeOfBound) {
			case RitterBoundingSphere:
				if (pObj->ritterBs[j].isOverlapFrustum(frustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::LightGreen);
					addToQ = 1;
				}
				else if (pObj->ritterBs[j].isInFrustum(frustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Blue);
					addToQ = 1;
				}
				else {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Red);
					//addToQ = 1; //debug
				}

				break;

			case NaiveBoundingSphere:
				if (pObj->naiveBs[j].isOverlapFrustum(frustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::LightGreen);
					addToQ = 1;
				}
				else if (pObj->naiveBs[j].isInFrustum(frustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Blue);
					addToQ = 1;
				}
				else {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Red);
					//addToQ = 1; //debug
				}

				break;

			case AABBBoundingBox:
				if (pObj->aabb[j].isOverlapFrustum(frustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::LightGreen);
					addToQ = 1;
				}
				else if (pObj->aabb[j].isInFrustum(frustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Blue);
					addToQ = 1;
				}
				else {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Red);
					//addToQ = 1; //debug
				}

				break;
			}

			if (addToQ) {
				// add to queue function
				if (isTransparent) {
					_toSortTransparentQ = true;

					_culledTransparentObjQ.push_back(std::make_pair(pObj, pMesh));

				} else {
					_culledOpaqueObjQ.push_back(std::make_pair(pObj, pMesh));
				}
			}
		}
	}
}

// Add random object with random transformations, meshes and textures
tre::Object* Scene::addRandomObj() {
	// Create new obj
	tre::Object newObj;

	int selectIdx = tre::Utility::getRandomInt(3);
	newObj.pObjMeshes = { &_debugMeshes[5 + selectIdx] };
	newObj.pObjMeshes[0]->pMaterial = &_debugMaterials[selectIdx];

	float scaleVal = tre::Utility::getRandomFloat(3);
	XMFLOAT3 objPos = tre::Maths::getRotatePosition(XMFLOAT3(.0f, .0f, .0f), .0f, tre::Utility::getRandomFloat(360.f), tre::Utility::getRandomFloat(100.f));
	newObj.objPos = objPos;
	newObj.objScale = XMFLOAT3(scaleVal, scaleVal, scaleVal);
	newObj.objRotation = XMFLOAT3(tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360));
	newObj._boundingVolumeColor = { tre::colorF(Colors::Green) };
	newObj.ritterBs = { newObj.pObjMeshes[0]->ritterSphere };
	newObj.naiveBs = { newObj.pObjMeshes[0]->naiveSphere };
	newObj.aabb = { newObj.pObjMeshes[0]->aabb };
	newObj._transformationFinal = tre::Maths::createTransformationMatrix(newObj.objScale, newObj.objRotation, newObj.objPos);

	_objQ.push_back(newObj);
	_pObjQ.push_back(&_objQ.back());

	return _pObjQ.back();
}

void Scene::update(const Graphics& graphics) {
	{	// Update Bounding volume for all objects once
		MICROPROFILE_SCOPE_CSTR("Update Bounding Volume");
		updateBoundingVolume(graphics.setting.typeOfBound);
	}
	{
		MICROPROFILE_SCOPE_CSTR("Update Directional Light Property");
		updateDirLight();
	}
}
}