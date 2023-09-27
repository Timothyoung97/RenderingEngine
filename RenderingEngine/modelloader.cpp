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
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace
	);

	CHECK_ERROR(pScene != nullptr, "File should be loaded");

	this->_directoryPath = tre::Utility::getDirPathStr(filename);

	this->processNode(device, pScene->mRootNode, pScene);
}

Texture ModelLoader::loadTextures(ID3D11Device* device, aiMaterial* mat, aiTextureType type, const aiScene* scene) {

	if (mat->GetTextureCount(type) == 0) {
		return Texture();
	}

	aiString str;
	mat->GetTexture(type, 0, &str); // hardcoded to get the first texture

	if (!_textures.contains(str.C_Str())) {
		std::string filename = this->_directoryPath + tre::Utility::uriDecode(std::string(str.C_Str()));
		_textures[str.C_Str()] = tre::TextureLoader::createTexture(device, filename);
	}

	return _textures[str.C_Str()];
}

void ModelLoader::processNode(ID3D11Device* device, aiNode* node, const aiScene* scene) {
	for (int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		if (!_meshes.contains(mesh->mName.C_Str())) {
			_meshes[mesh->mName.C_Str()] = CustomMesh(device, mesh);
		} 

		if (mesh->mMaterialIndex >= 0) {
			//_meshes[mesh->mName.C_Str()].material.objTexture = this->loadTextures(device, scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DIFFUSE, scene);
			//_meshes[mesh->mName.C_Str()].material.objNormalMap = this->loadTextures(device, scene->mMaterials[mesh->mMaterialIndex], aiTextureType_NORMALS, scene);
		}
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		this->processNode(device, node->mChildren[i], scene);
	}
};

}