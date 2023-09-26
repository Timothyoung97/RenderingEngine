#pragma once

#include "object.h"
#include "light.h"
#include "mesh.h"
#include "texture.h"

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
	std::vector<Mesh> meshes;
	std::vector<Texture> textures;
	std::vector<Texture> normalTextures;

	Scene(ID3D11Device* device);

	tre::Object floor;

	void createFloor();
	void updateDirLight();
};
}