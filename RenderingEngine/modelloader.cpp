#include "modelloader.h"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "assertm.h"
#include "utility.h"
#include "dxdebug.h"


namespace tre {

void ModelLoader::load(ID3D11Device* device, std::string filename) {
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_MakeLeftHanded | 
		aiProcess_FlipUVs
	);

	CHECK_ERROR(pScene != nullptr, "File should be loaded");

	this->_directoryPath = tre::Utility::getDirPathStr(filename);

	this->processNode(device, pScene->mRootNode, pScene);
}

void ModelLoader::loadTextures(ID3D11Device* device, aiMaterial* mat, aiTextureType type, const aiScene* scene) {
	
	for (int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);

		std::string filename = this->_directoryPath + std::string(str.C_Str());

		_textures.push_back(tre::TextureLoader::createTexture(device, filename));
	}
}

void ModelLoader::processNode(ID3D11Device* device, aiNode* node, const aiScene* scene) {
	for (int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		_meshes.push_back(CustomMesh(device, mesh));

		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			this->loadTextures(device, scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DIFFUSE, scene);
		}
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		this->processNode(device, node->mChildren[i], scene);
	}
};

}