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

	spdlog::info("Importing {}", filename);
	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_MakeLeftHanded | 
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace
	);

	CHECK_ERROR(pScene != nullptr, "File should be loaded");

	this->_directoryPath = tre::Utility::getDirPathStr(filename);

	spdlog::info("Importing Resources");
	this->loadResource(device, pScene);

	spdlog::info("Procesing Nodes");
	this->processNode(pScene->mRootNode, &this->_obj, nullptr, pScene);

	spdlog::info("Updating Transformation");
	this->updateObj(&this->_obj, XMMatrixIdentity());
}

void ModelLoader::loadResource(ID3D11Device* device, const aiScene* scene) {
	
	// first pass: load in all meshes, using its idx in scene as key
	spdlog::info("Begin Mesh Loading");
	for (int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		if (!_meshes.contains(i)) {
			spdlog::info("Loading mesh {}", i);
			_meshes[i] = CustomMesh(device, mesh);	
		}
	}

	// second pass: load in all textures via iterating through all materials, using filename as key
	spdlog::info("Begin Texture Loading");
	for (int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* pMaterial = scene->mMaterials[i];

		aiString diffuseTexName;

		pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexName);
		if (diffuseTexName.length && !_textures.contains(diffuseTexName.C_Str())) {
			spdlog::info("Loading diffuse texture {}", diffuseTexName.C_Str());
			std::string fullFilepath = this->_directoryPath + tre::Utility::uriDecode(std::string(diffuseTexName.C_Str()));
			_textures[diffuseTexName.C_Str()] = tre::TextureLoader::createTexture(device, fullFilepath);
		}

		aiString normalMapName;
		pMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalMapName);
		if (normalMapName.length && !_textures.contains(normalMapName.C_Str())) {
			spdlog::info("Loading normal texture {}", normalMapName.C_Str());
			std::string fullFilepath = this->_directoryPath + tre::Utility::uriDecode(std::string(normalMapName.C_Str()));
			_textures[normalMapName.C_Str()] = tre::TextureLoader::createTexture(device, fullFilepath);
		}
	}

	// assign respective materials with its textures
	spdlog::info("Assigning textures to materials");
	for (int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* pMaterial = scene->mMaterials[i];
		if (!_materials.contains(i)) {
			Material newMaterial{ nullptr, nullptr, tre::colorF(Colors::Black)};
			
			aiColor4D baseColor;
			pMaterial->Get(AI_MATKEY_BASE_COLOR, baseColor);
			if (!baseColor.IsBlack()) {
				newMaterial.baseColor = XMFLOAT4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
			}

			aiString diffuseTexName;
			pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexName);
			if (diffuseTexName.length != 0) {
				newMaterial.objTexture = &_textures[diffuseTexName.C_Str()];
			}

			aiString normalMapName;
			pMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalMapName);
			if (normalMapName.length != 0) {
				newMaterial.objNormalMap = &_textures[normalMapName.C_Str()];
			}

			_materials[i] = newMaterial;
		}
	}

	// assign materials to apply on mashes
	spdlog::info("Assigning materials to meshes");
	for (int i = 0; i < scene->mNumMeshes; i++) {
		_meshes[i].pMaterial = &_materials[scene->mMeshes[i]->mMaterialIndex];
	}
}

void ModelLoader::processNode(aiNode* currNode, Object* currObj, Object* pParent, const aiScene* scene) {

	currObj->parent = pParent;

	// compute transformation
	currObj->_transformationAssimp = currNode->mTransformation;

	memcpy(&currObj->_transformationFinal, &currObj->_transformationAssimp, sizeof(aiMatrix4x4));
	currObj->_transformationFinal = XMMatrixTranspose(currObj->_transformationFinal); // convert to coln major
	
	XMVECTOR scale, rotationQ, translation;
	XMMatrixDecompose(&scale, &rotationQ, &translation, currObj->_transformationFinal);

	XMStoreFloat3(&currObj->objScale, scale);
	XMStoreFloat3(&currObj->objPos, translation);

	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotationQ);
	currObj->objRotation = Maths::convertRotationMatrixToEuler(rotationMatrix);

	XMMATRIX scaleMatrix = XMMatrixScaling(currObj->objScale.x, currObj->objScale.y, currObj->objScale.z);
	XMMATRIX translationMatrix = XMMatrixTranslation(currObj->objPos.x, currObj->objPos.y, currObj->objPos.z);

	currObj->_transformationFinal = scaleMatrix * rotationMatrix * translationMatrix;

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

// Update Obj by decomposing assimp imported transformation
void ModelLoader::updateObj(Object* _obj, XMMATRIX cumulativeMatrix) {

	_obj->_transformationFinal = _obj->_transformationFinal * cumulativeMatrix;

	for (int i = 0; i < _obj->children.size(); i++) {
		updateObj(&_obj->children[i], _obj->_transformationFinal);
	}
}

// Update Obj directly using aiMatrix4x4
void ModelLoader::updateObj(Object* _obj, aiMatrix4x4 cumulativeMatrix) {
	_obj->_transformationAssimp = cumulativeMatrix * _obj->_transformationAssimp;

	memcpy(&_obj->_transformationFinal, &_obj->_transformationAssimp, sizeof(aiMatrix4x4));

	_obj->_transformationFinal = XMMatrixTranspose(_obj->_transformationFinal);

	for (int i = 0; i < _obj->children.size(); i++) {
		updateObj(&_obj->children[i], _obj->_transformationAssimp);
	}
}

}