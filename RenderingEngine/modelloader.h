#pragma once

#include <d3d11.h>
#include <assimp/scene.h>

#include <vector>

#include <mesh.h>

namespace tre {

class ModelLoader {
public:

	std::vector<Mesh> _meshes;

	void load(ID3D11Device* device, std::string filename);
	void processNode(ID3D11Device* device, aiNode* node, const aiScene* scene);
};
}