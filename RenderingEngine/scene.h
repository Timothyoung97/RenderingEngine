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

static const float BACKGROUND_COLOR[4] = { .5f, .5f, .5f, 1.0f };

class Scene {
public:
	// Directional light
	tre::Light dirlight;
	float dirlightYaw = .0f;
	float dirlightPitch = 45.f;
	
	//Pt Light
	tre::LightResource lightResc;

	// Resources
	std::vector<Mesh> _debugMeshes;
	std::vector<Texture> _debugTextures;
	std::vector<Texture> _debugNormalTextures;
	std::vector<Material> _debugMaterials;

	// Object Queue to store debug objects created in Scene
	std::vector<tre::Object> _objQ;

	// Object ptr queue
	std::vector<tre::Object*> _pObjQ;

	// Render Queue
	std::vector<std::pair<tre::Object*, tre::Mesh*>> _culledOpaqueObjQ;
	std::vector<std::pair<tre::Object*, tre::Mesh*>> _culledTransparentObjQ;
	std::vector<std::pair<tre::Object*, tre::Mesh*>> _wireframeObjQ;

	bool _toSortTransparentQ = false;
	bool _toRecalDistFromCam = false;

	Scene(ID3D11Device* device);

	tre::Object _floor;

	void createFloor();
	void updateDirLight();
	void updateBoundingVolume(BoundVolumeEnum typeOfBound);
	void cullObject(Camera& cam, BoundVolumeEnum typeOfBound);
	void updateTransparentQ(Camera& cam);
};
}