#pragma once

#include "object.h"

namespace tre {
class Scene {
public:

	std::vector<Mesh> meshes;
	std::vector<Texture> textures;
	std::vector<Texture> normalTextures;

	Scene(ID3D11Device* device);

	tre::Object floor;

	void createFloor();
};
}