#pragma once

#include "object.h"

namespace tre {
class Scene {
public:
	//Background Color
	const float bgColor[4] = { .5f, .5f, .5f, 1.0f };

	std::vector<Mesh> meshes;
	std::vector<Texture> textures;
	std::vector<Texture> normalTextures;


	Scene(ID3D11Device* device);

	tre::Object floor;

	void createFloor();
};
}