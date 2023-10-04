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
		dirF, .0f, XMFLOAT4(.5f, .5f, .5f, 1.0f), XMFLOAT4(.5f, .5f, .5f, .5f)
	};
}

void Scene::updateBoundingVolume(BoundVolumeEnum typeOfBound) {
	for (int i = 0; i < _pObjQ.size(); i++) {
		Object* pObj = _pObjQ[i];
		for (int j = 0; j < pObj->pObjMeshes.size(); j++) {
			switch (typeOfBound) {
			case tre::AABBBoundingBox:
				tre::BoundingVolume::updateAABB(pObj->pObjMeshes[j]->aabb, pObj->aabb[j], pObj->_transformationFinal);
				break;
			case tre::RitterBoundingSphere:
				tre::BoundingVolume::updateBoundingSphere(pObj->pObjMeshes[j]->ritterSphere, pObj->ritterBs[j], pObj->_transformationFinal);
				break;
			case tre::NaiveBoundingSphere:
				tre::BoundingVolume::updateBoundingSphere(pObj->pObjMeshes[j]->naiveSphere, pObj->naiveBs[j], pObj->_transformationFinal);
				break;
			}
		}
	}
}

void Scene::cullObject(Camera& cam, BoundVolumeEnum typeOfBound) {

	updateBoundingVolume(typeOfBound);

	_culledOpaqueObjQ.clear();
	_culledOpaqueObjQ.push_back(std::make_pair(&_floor, _floor.pObjMeshes[0]));
	_culledTransparentObjQ.clear();

	for (int i = 0; i < _pObjQ.size(); i++) {
		Object* pObj = _pObjQ[i];
		for (int j = 0; j < pObj->pObjMeshes.size(); j++) {
			Mesh* pMesh = pObj->pObjMeshes[j];

			int isTransparent = 0;
			if ((pMesh->material->objTexture != nullptr && pMesh->material->objTexture->hasAlphaChannel)
				|| (pMesh->material->objTexture == nullptr && pMesh->material->baseColor.w < 1.0f)) {

				isTransparent = 1;
			}

			int addToQ = 0;
			switch (typeOfBound) {
			case RitterBoundingSphere:
				if (pObj->ritterBs[j].isOverlapFrustum(cam.cameraFrustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::LightGreen);
					addToQ = 1;
				}
				else if (pObj->ritterBs[j].isInFrustum(cam.cameraFrustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Blue);
					addToQ = 1;
				}
				else {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Red);
				}

				break;

			case NaiveBoundingSphere:
				if (pObj->naiveBs[j].isOverlapFrustum(cam.cameraFrustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::LightGreen);
					addToQ = 1;
				}
				else if (pObj->naiveBs[j].isInFrustum(cam.cameraFrustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Blue);
					addToQ = 1;
				}
				else {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Red);
				}

				break;

			case AABBBoundingBox:
				if (pObj->aabb[j].isOverlapFrustum(cam.cameraFrustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::LightGreen);
					addToQ = 1;
				}
				else if (pObj->aabb[j].isInFrustum(cam.cameraFrustum)) {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Blue);
					addToQ = 1;
				}
				else {
					pObj->_boundingVolumeColor[j] = tre::colorF(Colors::Red);
				}

				break;
			}

			if (addToQ) {
				// add to queue function
				if (isTransparent) {
					// find its distance from cam
					if (_toRecalDistFromCam) {
						pObj->distFromCam = tre::Maths::distBetweentObjToCam(pObj->objPos, cam.camPositionV);
					}
					_toSortTransparentQ = true;

					_culledTransparentObjQ.push_back(std::make_pair(pObj, pMesh));

				} else {
					_culledOpaqueObjQ.push_back(std::make_pair(pObj, pMesh));
				}

			}

		}
	}

	if (_toRecalDistFromCam) {
		_toRecalDistFromCam = false;
	}

	// sort the vector -> object with greater dist from cam is at the front of the Q
	if (_toSortTransparentQ) {
		std::sort(_culledTransparentObjQ.begin(), _culledTransparentObjQ.end(), [](const std::pair<tre::Object*, tre::Mesh*> obj1, const std::pair<tre::Object*, tre::Mesh*> obj2) { return obj1.first->distFromCam > obj2.first->distFromCam; });
		_toSortTransparentQ = false;
	}
}

}