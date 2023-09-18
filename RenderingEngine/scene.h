#pragma once

#include "object.h"
#include "light.h"
#include "mesh.h"
#include "texture.h"


#include <vector>

namespace tre {
class Scene {
public:
	//Background Color
	const float bgColor[4] = { .5f, .5f, .5f, 1.0f };

	// Directional light
	tre::Light dirlight;


	std::vector<Mesh> meshes;
	std::vector<Texture> textures;
	std::vector<Texture> normalTextures;

	Scene(ID3D11Device* device);

	tre::Object floor;

	void createFloor();
	void createDirLight();
};
}