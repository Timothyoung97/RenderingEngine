#pragma once

#include "object.h"
#include "light.h"
#include "mesh.h"
#include "texture.h"
#include "material.h"
#include "light.h"
#include "camera.h"

#include <vector>

namespace tre {

static const float BACKGROUND_GREY[4] = { .5f, .5f, .5f, 1.f };
static const float BACKGROUND_BLACK[4] = { .0f, .0f, .0f, 1.f };

class Scene {
public:
	// Directional light
	tre::Light dirlight;
	float dirlightYaw = .0f;
	float dirlightPitch = 45.f;
	float dirLightDiffuse = .5f;
	
	//Pt Light
	tre::LightResource lightResc;

	// Resources
	std::vector<Mesh> _debugMeshes;
	std::vector<Texture> _debugTextures;
	std::vector<Texture> _debugNormalTextures;
	std::vector<Material> _debugMaterials;

	// Object Queue to store debug objects created in Scene
	std::vector<tre::Object> _objQ;
	std::vector<tre::Object> _pointLightObjQ;

	// Object ptr queue
	std::vector<tre::Object*> _pObjQ;

	// Render Queue
	std::vector<std::pair<tre::Object*, tre::Mesh*>> _culledOpaqueObjQ;
	std::vector<std::pair<tre::Object*, tre::Mesh*>> _culledTransparentObjQ;
	std::vector<std::pair<tre::Object*, tre::Mesh*>> _wireframeObjQ;

	bool _toSortTransparentQ = false;
	bool _toRecalDistFromCam = true; // when scene is init, to calculate distance from camera

	Scene(ID3D11Device* device, ID3D11DeviceContext* context);

	tre::Object _floor;

	void createFloor();
	void updateDirLight();
	void updateBoundingVolume(BoundVolumeEnum typeOfBound);
	void cullObject(Frustum& frustum, BoundVolumeEnum typeOfBound);
	void updateTransparentQ(Camera& cam);

	tre::Object* addRandomObj();
};
}