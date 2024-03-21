#include "scene.h"

#include <format>
#include <taskflow/taskflow.hpp>

#include "microprofile.h"
#include "colors.h"
#include "utility.h"
#include "window.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

Scene::Scene() {
	this->init();
}

void Scene::init() {

	this->_objQ.reserve(1000000); // hardcoded capacity

	_debugMeshes = {
		tre::CubeMesh(pEngine->device->device.Get()), // Bounding WireMesh
		tre::SphereMesh(pEngine->device->device.Get(), 20, 20), // Bounding WireMesh
		tre::TeapotMesh(pEngine->device->device.Get()),
		tre::CubeMesh(pEngine->device->device.Get()), // floor
		tre::CubeMesh(pEngine->device->device.Get()), // transparent cube
		tre::CubeMesh(pEngine->device->device.Get()), // testing cube
		tre::SphereMesh(pEngine->device->device.Get(), 6, 6), // testing sphere
		tre::CubeMesh(pEngine->device->device.Get()), // testing cube
		tre::SphereMesh(pEngine->device->device.Get(), 6, 6), // testing sphere
	};
	
	// Create testing texture
	std::string basePathStr = Utility::getBasePathStr();
	_debugTextures = {
		tre::TextureLoader::createTexture(pEngine->device->device.Get(), basePathStr + "textures\\UV_image.jpg"),
		tre::TextureLoader::createTexture(pEngine->device->device.Get(), basePathStr + "textures\\UV_image2.jpg"),
		tre::TextureLoader::createTexture(pEngine->device->device.Get(), basePathStr + "textures\\UV_image_a.png"),
		tre::TextureLoader::createTexture(pEngine->device->device.Get(), basePathStr + "textures\\glTF.png"),
		tre::TextureLoader::createTexture(pEngine->device->device.Get(), basePathStr + "textures\\wall.jpg"),
	};

	_debugNormalTextures = {
		tre::TextureLoader::createTexture(pEngine->device->device.Get(), basePathStr + "textures\\glTF_normal.png"),
		tre::TextureLoader::createTexture(pEngine->device->device.Get(), basePathStr + "textures\\wall_normal.jpg")
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
	lightResc.create(pEngine->device->device.Get(), pEngine->device->contextI.Get());
	
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
	_floor.isInView = { true };
	_floor._boundingVolumeColor = { tre::colorF(Colors::WhiteSmoke) };
	_floor._transformationFinal = tre::Maths::createTransformationMatrix(_floor.objScale, _floor.objRotation, _floor.objPos);
	_pObjQ.push_back(&_floor);
}

void Scene::createDebugObject() {
	tre::Object debugModel;
	debugModel.pObjMeshes = { &_debugMeshes[4] };
	debugModel.pObjMeshes[0]->pMaterial = &_debugMaterials[5];
	debugModel.objPos = XMFLOAT3(.0f, .5f, .0f);
	debugModel.objScale = XMFLOAT3(1.f, 1.f, 1.f);
	debugModel.objRotation = XMFLOAT3(.0f, .0f, .0f);
	debugModel.ritterBs = { debugModel.pObjMeshes[0]->ritterSphere };
	debugModel.naiveBs = { debugModel.pObjMeshes[0]->naiveSphere };
	debugModel.aabb = { debugModel.pObjMeshes[0]->aabb };
	debugModel._boundingVolumeColor = { tre::colorF(Colors::LightGreen) };
	debugModel.isInView = { true };
	_objQ.push_back(debugModel);
	_pObjQ.push_back(&_objQ.back());
	_debugObject = _pObjQ.back();
}

void Scene::createViewProjections(const Graphics& graphics, const Camera& cam) {
	MICROPROFILE_SCOPE_CSTR("Update View Projections");

	viewProjs.clear();
	viewProjs.push_back(cam.camViewProjection); // first push in camera's view projections

	for (int i = 0; i < 4; i++) { // for 4 quads

		MICROPROFILE_SCOPE_CSTR("Build CSM View Projection Matrix");

		// projection matrix of camera with specific near and far plane
		XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, graphics.setting.csmPlaneIntervals[i], graphics.setting.csmPlaneIntervals[i + 1]);

		std::vector<XMVECTOR> corners = tre::Maths::getFrustumCornersWorldSpace(XMMatrixMultiply(cam.camView, projMatrix));

		XMVECTOR center = tre::Maths::getAverageVector(corners);

		XMMATRIX lightView = XMMatrixLookAtLH(center + XMVECTOR{ dirlight.direction.x, dirlight.direction.y, dirlight.direction.z }, center, XMVECTOR{ .0f, 1.f, .0f });

		XMMATRIX lightOrthoProj = tre::Maths::createOrthoMatrixFromFrustumCorners(10.f, corners, lightView);

		XMMATRIX lightViewProj = XMMatrixMultiply(lightView, lightOrthoProj);

		viewProjs.push_back(lightViewProj);
	}
}

void Scene::updateDirLight() {
	MICROPROFILE_SCOPE_CSTR("Update Directional Light Property");

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
		tre::ObjectUtility::updateBoundingVolumeTransformation(*pObj, typeOfBound);
	}
}

void Scene::updateCulledOpaqueQ() {
	MICROPROFILE_SCOPE_CSTR("Grouping Opaque Objects");

	for (int i = 0; i < _culledOpaqueObjQ.size(); i++) {

		// Grouping first based on mesh, then based on textures
		std::sort(_culledOpaqueObjQ[i].begin(), _culledOpaqueObjQ[i].end(),
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
}

void Scene::updateCulledTransparentQ(const Camera& cam) {
	MICROPROFILE_SCOPE_CSTR("Sorting Transparent Object Based on Depth");

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

void Scene::cullObject(std::vector<Frustum>& frustums, BoundVolumeEnum typeOfBound) {

	MICROPROFILE_SCOPE_CSTR("Cull Object Based on Views");

	// clear render queue
	_culledOpaqueObjQ.clear();
	_culledTransparentObjQ.clear();

	for (int viewIdx = 0; viewIdx < frustums.size(); viewIdx++) {
		MICROPROFILE_SCOPE_CSTR("Cull View");
		for (int i = 0; i < _pObjQ.size(); i++) {
			Object* pObj = _pObjQ[i];
			for (int j = 0; j < pObj->pObjMeshes.size(); j++) {
				Mesh* pMesh = pObj->pObjMeshes[j];

				bool isTransparent = pMesh->pMaterial->isTransparent();
				bool addToQ = tre::ObjectUtility::isMeshWithinView(*pObj, j, frustums[viewIdx], typeOfBound, viewIdx == camViewIdx);

				if (!addToQ) continue;

				if (isTransparent && viewIdx == 0) {
					_toSortTransparentQ = true;
					_culledTransparentObjQ.push_back(std::make_pair(pObj, pMesh));
					continue;
				}

				if (isTransparent) continue;

				_culledOpaqueObjQ[viewIdx].push_back(std::make_pair(pObj, pMesh));
			}
		}

	}
}

void Scene::update(const Graphics& graphics, const Camera& cam) {
	MICROPROFILE_SCOPE_CSTR("Scene Update");

	createViewProjections(graphics, cam);
	updateBoundingVolume(graphics.setting.typeOfBound);
	updateDirLight();

	std::vector<Frustum> frustums;
	for (const XMMATRIX& viewProj : viewProjs) {
		frustums.push_back(tre::Maths::createFrustumFromViewProjectionMatrix(viewProj));
	}
	
	cullObject(frustums, graphics.setting.typeOfBound);
	updateCulledOpaqueQ();
	updateCulledTransparentQ(cam);
	updatePtLight();
}

void Scene::updatePtLight() {
	MICROPROFILE_SCOPE_CSTR("CPU Point Light Update");
	//PROFILE_GPU_SCOPED("CPU Point Light Update");

	lightResc.updatePtLightCPU();
	_pointLightObjQ.clear();
	_pointLightObjQ.reserve(lightResc.readOnlyPointLightQ.size());
	_wireframeObjQ.clear();
	for (int i = 0; i < lightResc.readOnlyPointLightQ.size(); i++) {
		tre::Object newLightObj;

		newLightObj.pObjMeshes = { &_debugMeshes[1] }; // sphere
		newLightObj.pObjMeshes[0]->pMaterial = &_debugMaterials[2];
		newLightObj.objPos = lightResc.readOnlyPointLightQ[i].pos;
		newLightObj.objScale = XMFLOAT3(lightResc.readOnlyPointLightQ[i].range, lightResc.readOnlyPointLightQ[i].range, lightResc.readOnlyPointLightQ[i].range);
		newLightObj.objRotation = XMFLOAT3(.0f, .0f, .0f);
		newLightObj._boundingVolumeColor = { tre::colorF(Colors::White) };
		newLightObj._transformationFinal = tre::Maths::createTransformationMatrix(newLightObj.objScale, newLightObj.objRotation, newLightObj.objPos);
		newLightObj._boundingVolumeTransformation.push_back(newLightObj._transformationFinal);
		newLightObj.isInView = { true };

		_pointLightObjQ.push_back(newLightObj);
		_wireframeObjQ.push_back(&_pointLightObjQ.back());
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
	newObj.isInView = { true };
	newObj.ritterBs = { newObj.pObjMeshes[0]->ritterSphere };
	newObj.naiveBs = { newObj.pObjMeshes[0]->naiveSphere };
	newObj.aabb = { newObj.pObjMeshes[0]->aabb };
	newObj._transformationFinal = tre::Maths::createTransformationMatrix(newObj.objScale, newObj.objRotation, newObj.objPos);

	_objQ.push_back(newObj);
	_pObjQ.push_back(&_objQ.back());

	return _pObjQ.back();
}

}