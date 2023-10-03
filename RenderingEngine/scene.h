#pragma once

#include "object.h"
#include "light.h"
#include "mesh.h"
#include "texture.h"
#include "material.h"

#include <vector>

namespace tre {

static const float BACKGROUND_COLOR[4] = { .5f, .5f, .5f, 1.0f };

class Scene {
public:
	// Directional light
	tre::Light dirlight;
	float dirlightYaw = .0f;
	float dirlightPitch = 45.f;
	
	// Resources
	std::vector<Mesh> _debugMeshes;
	std::vector<Texture> _debugTextures;
	std::vector<Texture> _debugNormalTextures;
	std::vector<Material> _debugMaterials;

	// Object Q
	std::vector<tre::Object> _opaqueObjQ;
	std::vector<tre::Object> _transparentObjQ;
	std::vector<tre::Object> _culledOpaqueObjQ;
	std::vector<tre::Object> _culledTransparentObjQ;

	bool _toSortTransparentQ = false;
	bool _toRecalDistFromCam = false;

	Scene(ID3D11Device* device);

	tre::Object _floor;

	void createFloor();
	void updateDirLight();
};
}