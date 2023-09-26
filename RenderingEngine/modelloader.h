#pragma once

#include <d3d11.h>
#include <assimp/scene.h>

#include <vector>

#include "mesh.h"
#include "texture.h"

namespace tre {

class ModelLoader {
public:

	std::vector<Mesh> _meshes;
	std::vector<Texture> _textures;
	std::vector<Material> _materials;
	std::string _directoryPath;

	void load(ID3D11Device* device, std::string filename);
	Texture loadTextures(ID3D11Device* device, aiMaterial* mat, aiTextureType type, const aiScene* scene);
	void processNode(ID3D11Device* device, aiNode* node, const aiScene* scene);
};
}