#include "modelloader.h"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <DirectXMath.h>

#include "assertm.h"
#include "utility.h"
#include "dxdebug.h"
#include "colors.h"
#include "maths.h"

using namespace DirectX;

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

	this->loadResource(device, pScene);

	this->processNode(pScene->mRootNode, &this->_obj, nullptr, pScene);

	this->updateObj(&this->_obj, aiMatrix4x4());
}

void ModelLoader::loadResource(ID3D11Device* device, const aiScene* scene) {
	
	// first pass: load in all meshes, using its idx in scene as key
	for (int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		if (!_meshes.contains(i)) {
			_meshes[i] = CustomMesh(device, mesh);	
		}
	}

	// second pass: load in all textures via iterating through all materials, using filename as key
	for (int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* material = scene->mMaterials[i];

		aiString diffuseTexName;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexName);
		if (diffuseTexName.length && !_textures.contains(diffuseTexName.C_Str())) {
			std::string fullFilepath = this->_directoryPath + tre::Utility::uriDecode(std::string(diffuseTexName.C_Str()));
			_textures[diffuseTexName.C_Str()] = tre::TextureLoader::createTexture(device, fullFilepath);
		}

		aiString normalMapName;
		material->GetTexture(aiTextureType_NORMALS, 0, &normalMapName);
		if (normalMapName.length && !_textures.contains(normalMapName.C_Str())) {
			std::string fullFilepath = this->_directoryPath + tre::Utility::uriDecode(std::string(normalMapName.C_Str()));
			_textures[normalMapName.C_Str()] = tre::TextureLoader::createTexture(device, fullFilepath);
		}
	}

	// assign respective materials with its textures
	for (int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* material = scene->mMaterials[i];
		if (!_materials.contains(i)) {
			Material newMaterial{ nullptr, nullptr, tre::colorF(Colors::Black)};
			
			aiColor4D baseColor;
			material->Get(AI_MATKEY_BASE_COLOR, baseColor);
			if (!baseColor.IsBlack()) {
				newMaterial.baseColor = XMFLOAT4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
			}

			aiString diffuseTexName;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexName);
			if (diffuseTexName.length != 0) {
				newMaterial.objTexture = &_textures[diffuseTexName.C_Str()];
			}

			aiString normalMapName;
			material->GetTexture(aiTextureType_NORMALS, 0, &normalMapName);
			if (normalMapName.length != 0) {
				newMaterial.objTexture = &_textures[normalMapName.C_Str()];
			}

			_materials[i] = newMaterial;
		}
	}

	// assign materials to apply on mashes
	for (int i = 0; i < scene->mNumMeshes; i++) {
		_meshes[i].material = &_materials[scene->mMeshes[i]->mMaterialIndex];
	}
}

void ModelLoader::processNode(aiNode* currNode, Object* currObj, Object* pParent, const aiScene* scene) {

	aiVector3D position, eularRotation, scale;
	currNode->mTransformation.Decompose(scale, eularRotation, position);
	currObj->objPos = XMFLOAT3(position.x, position.y, position.z);
	currObj->objRotation = XMFLOAT3(XMConvertToDegrees(eularRotation.x), XMConvertToDegrees(eularRotation.y), XMConvertToDegrees(eularRotation.z));
	currObj->objScale = XMFLOAT3(scale.x, scale.y, scale.z);
	currObj->_transformationFinal = Maths::createTransformationMatrix(currObj->objScale, currObj->objRotation, currObj->objPos);
	currObj->_transformationAssimp = currNode->mTransformation;

	currObj->parent = pParent;
	
	// check if this node has mesh, if there is assign one first
	if (currNode->mNumMeshes != 0) {
		for (int i = 0; i < currNode->mNumMeshes; i++) {

			currObj->pObjMeshes.push_back(&_meshes[currNode->mMeshes[i]]);
			currObj->aabb.push_back(currObj->pObjMeshes.back()->aabb);
			currObj->ritterBs.push_back(currObj->pObjMeshes.back()->ritterSphere);
			currObj->naiveBs.push_back(currObj->pObjMeshes.back()->naiveSphere);
			currObj->_boundingVolumeColor.push_back(tre::colorF(Colors::LightGreen));
		}
	}

	// recurse down to children
	if (currNode->mNumChildren != 0) {
		currObj->children.reserve(currNode->mNumChildren);
		for (int i = 0; i < currNode->mNumChildren; i++) {
			currObj->children.push_back(Object());
			this->processNode(currNode->mChildren[i], &currObj->children[i], currObj, scene);
		}
	}

	if (currNode->mNumMeshes != 0) {
		_objectWithMesh.push_back(currObj);
	}
};


void ModelLoader::updateObj(Object* _obj, aiMatrix4x4 cumulativeMatrix) {
	_obj->_transformationAssimp = cumulativeMatrix * _obj->_transformationAssimp;

	memcpy(&_obj->_transformationFinal, &_obj->_transformationAssimp, sizeof(aiMatrix4x4));

	_obj->_transformationFinal = XMMatrixTranspose(_obj->_transformationFinal);

	for (int i = 0; i < _obj->children.size(); i++) {
		updateObj(&_obj->children[i], _obj->_transformationAssimp);
	}
}

}