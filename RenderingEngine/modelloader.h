#pragma once

#include <d3d11.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <DirectXMath.h>

#include <unordered_map>
#include <vector>

#include "mesh.h"
#include "texture.h"
#include "object.h"
#include "assertm.h"
#include "utility.h"
#include "dxdebug.h"
#include "colors.h"
#include "maths.h"

namespace tre {

class ModelLoader {
public:
	std::unordered_map<int, Mesh> _meshes;
	std::unordered_map<std::string, Texture> _textures;
	std::unordered_map<int, Material> _materials;
	
	std::vector<Object*> _objectWithMesh;

	Object _obj;

	std::string _directoryPath;

	void load(ID3D11Device* device, std::string filename);
	void loadResource(ID3D11Device* device, const aiScene* scene);
	void processNode(aiNode* currNode, Object* currObj, Object* pParent, const aiScene* scene);
	void updateObj(Object* _obj, aiMatrix4x4 cumulativeMatrix);
	void updateObj(Object* _obj, XMMATRIX cumulativeMatrix);
};
}