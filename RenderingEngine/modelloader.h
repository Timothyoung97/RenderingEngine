#pragma once

#include <d3d11.h>
#include <assimp/scene.h>

#include <unordered_map>

#include "mesh.h"
#include "texture.h"

namespace tre {

class ModelLoader {
public:

	std::unordered_map<std::string, Mesh> _meshes;
	std::unordered_map<std::string, Texture> _textures;
	std::unordered_map<std::string, Material> _materials;
	std::string _directoryPath;

	void load(ID3D11Device* device, std::string filename);
	void loadResource(ID3D11Device* device, const aiScene* scene);
	Texture loadTextures(ID3D11Device* device, aiMaterial* mat, aiTextureType type, const aiScene* scene);
	void processNode(ID3D11Device* device, aiNode* node, const aiScene* scene);
};
}