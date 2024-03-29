#include "instanceBuffer.h"

namespace tre {

void InstanceBuffer::createBuffer(ID3D11Device* pDevice) {

	_device = pDevice;

	 //Persistent Buffer
	D3D11_BUFFER_DESC pInstanceBufferDesc;
	pInstanceBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	pInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	pInstanceBufferDesc.CPUAccessFlags = 0u;
	pInstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pInstanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * currMaxInstanceCount); //hardcoded initial size
	pInstanceBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(InstanceInfo));

	CHECK_DX_ERROR(_device->CreateBuffer(
		&pInstanceBufferDesc, nullptr, pInstanceBuffer.GetAddressOf()
	));
}

InstanceInfo InstanceBuffer::createInstanceInfo(XMMATRIX transformationLocal, XMFLOAT4 color, UINT isWithTexture, UINT hasNormalMap) {
	InstanceInfo newInstInfo;
	newInstInfo.transformation = transformationLocal;

	XMMATRIX normalMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, newInstInfo.transformation));
	XMFLOAT3X3 normalFloat3x3;
	XMStoreFloat3x3(&normalFloat3x3, normalMatrix);
	newInstInfo.normalMatrix = XMLoadFloat3x3(&normalFloat3x3);

	newInstInfo.isWithTexture = isWithTexture;
	newInstInfo.hasNormMap = hasNormalMap;
	newInstInfo.color = color;

	return newInstInfo;
};


int InstanceBuffer::updateBuffer(const std::vector<std::pair<Object*, Mesh*>>& objQ, ID3D11DeviceContext* deferredContext) {

	MICROPROFILE_SCOPE_CSTR("Batching Instances");
	//PROFILE_GPU_SCOPED("GPU Batching Instances");

	std::vector<InstanceInfo> instanceInfoQ;
	instanceBatchQueue.clear();

	int batchStartIdx = 0;
	Mesh* pBatchMesh = objQ[0].second;
	Texture* pBatchTexture = objQ[0].second->pMaterial->objTexture;
	Texture* pBatchNormalMap = objQ[0].second->pMaterial->objNormalMap;
	
	for (int i = 0; i < objQ.size() + 1; i++) {

		// For the final batch
		if (i == objQ.size()) {
			InstanceBatchInfo newBatchInfo = { 
				batchStartIdx, i - batchStartIdx,
				pBatchTexture != nullptr ? 1 : 0, pBatchNormalMap != nullptr ? 1 : 0, 
				pBatchMesh, pBatchTexture, pBatchNormalMap 
			};
			instanceBatchQueue.push_back(newBatchInfo);
			break;
		}

		// Each instance's information
		Object* pObj = objQ[i].first;
		Mesh* pMesh = objQ[i].second;
		Texture* pTexture = pMesh->pMaterial->objTexture;
		Texture* pNormalMap = pMesh->pMaterial->objNormalMap;
		
		// Push each instance's information to the vector
		InstanceInfo newInstInfo = this->createInstanceInfo(pObj->_transformationFinal, pMesh->pMaterial->baseColor, pTexture != nullptr ? 1 : 0, pNormalMap != nullptr ? 1 : 0);
		instanceInfoQ.push_back(newInstInfo);

		// If different mesh or different texture, a batch is formed
		if (pMesh != pBatchMesh || pTexture != pBatchTexture) {

			// push in previous batch
			InstanceBatchInfo newBatchInfo = { 
				batchStartIdx, i - batchStartIdx, 
				pBatchTexture != nullptr ? 1 : 0, pBatchNormalMap != nullptr ? 1 : 0, 
				pBatchMesh, pBatchTexture, pBatchNormalMap 
			};
			instanceBatchQueue.push_back(newBatchInfo);

			// assign new batch information
			pBatchMesh = pMesh;
			pBatchTexture = pTexture;
			pBatchNormalMap = pNormalMap;
			batchStartIdx = i;
		}

	}
	
	// Staging Buffer
	D3D11_BUFFER_DESC stagingInstancedBufferDesc;
	stagingInstancedBufferDesc.BindFlags = 0u;
	stagingInstancedBufferDesc.Usage = D3D11_USAGE_STAGING;
	stagingInstancedBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingInstancedBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	stagingInstancedBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * instanceInfoQ.size());
	stagingInstancedBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(tre::InstanceInfo));

	D3D11_SUBRESOURCE_DATA pInstanceBufferData = {};
	pInstanceBufferData.pSysMem = instanceInfoQ.data();

	ComPtr<ID3D11Buffer> pStagingInstanceBuffer;
	CHECK_DX_ERROR(_device->CreateBuffer(
		&stagingInstancedBufferDesc, &pInstanceBufferData, &pStagingInstanceBuffer
	));

	// If structured buffer on GPU has insufficient size
	if (currMaxInstanceCount < instanceInfoQ.size()) {
		currMaxInstanceCount = instanceInfoQ.size() * 2;
		pInstanceBuffer.Reset();

		// GPU Buffer
		D3D11_BUFFER_DESC pInstanceBufferDesc;
		pInstanceBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		pInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		pInstanceBufferDesc.CPUAccessFlags = 0u;
		pInstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		pInstanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * currMaxInstanceCount);
		pInstanceBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(tre::InstanceInfo));

		CHECK_DX_ERROR(_device->CreateBuffer(
			&pInstanceBufferDesc, nullptr, pInstanceBuffer.GetAddressOf()
		));		
	}

	deferredContext->CopySubresourceRegion(pInstanceBuffer.Get(), 0u, 0u, 0u, 0u, pStagingInstanceBuffer.Get(), 0u, nullptr);

	return instanceInfoQ.size();
}

int InstanceBuffer::updateBuffer(const std::vector<Object*>& objQ, Mesh* specifiedMesh, ID3D11DeviceContext* deferredContext) {

	if (objQ.empty()) return 0;

	MICROPROFILE_SCOPE_CSTR("Batching Instances Wireframe");
	//PROFILE_GPU_SCOPED("GPU Batching Instances Wireframe");

	std::vector<InstanceInfo> instanceInfoQ;
	instanceBatchQueue.clear();

	std::set<Object*> processed;
	int quantity = 0;
	for (int i = 0; i < objQ.size(); i++) {
		// Each instance's information
		Object* pObj = objQ[i];
		if (processed.contains(pObj)) {
			continue;
		}

		processed.insert(pObj);
		for (int j = 0; j < pObj->_boundingVolumeTransformation.size(); j++) {
			// Push each instance's information to the vector
			if (!pObj->isInView[j]) {
				continue;
			}
			InstanceInfo newInstInfo = this->createInstanceInfo(pObj->_boundingVolumeTransformation[j], pObj->_boundingVolumeColor[j], 0u, 0u);
			instanceInfoQ.push_back(newInstInfo);
			quantity++;
		}
	}

	if (!quantity) return 0;

	InstanceBatchInfo singleBatch;
	singleBatch.batchStartIdx = 0;
	singleBatch.hasNormMap = 0;
	singleBatch.isWithTexture = 0;
	singleBatch.pBatchMesh = specifiedMesh;
	singleBatch.pBatchNormalMap = nullptr;
	singleBatch.pBatchTexture = nullptr;
	singleBatch.quantity = quantity;
	instanceBatchQueue.push_back(singleBatch);

	// Staging Buffer
	D3D11_BUFFER_DESC stagingInstancedBufferDesc;
	stagingInstancedBufferDesc.BindFlags = 0u;
	stagingInstancedBufferDesc.Usage = D3D11_USAGE_STAGING;
	stagingInstancedBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingInstancedBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	stagingInstancedBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * instanceInfoQ.size());
	stagingInstancedBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(tre::InstanceInfo));

	D3D11_SUBRESOURCE_DATA pInstanceBufferData = {};
	pInstanceBufferData.pSysMem = instanceInfoQ.data();

	ComPtr<ID3D11Buffer> pStagingInstanceBuffer;
	CHECK_DX_ERROR(_device->CreateBuffer(
		&stagingInstancedBufferDesc, &pInstanceBufferData, &pStagingInstanceBuffer
	));

	// If structured buffer on GPU has insufficient size
	if (currMaxInstanceCount < instanceInfoQ.size()) {
		currMaxInstanceCount = instanceInfoQ.size() * 2;
		pInstanceBuffer.Reset();

		// GPU Buffer
		D3D11_BUFFER_DESC pInstanceBufferDesc;
		pInstanceBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		pInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		pInstanceBufferDesc.CPUAccessFlags = 0u;
		pInstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		pInstanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * currMaxInstanceCount);
		pInstanceBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(tre::InstanceInfo));

		CHECK_DX_ERROR(_device->CreateBuffer(
			&pInstanceBufferDesc, nullptr, pInstanceBuffer.GetAddressOf()
		));
	}

	deferredContext->CopySubresourceRegion(pInstanceBuffer.Get(), 0u, 0u, 0u, 0u, pStagingInstanceBuffer.Get(), 0u, nullptr);

	return instanceInfoQ.size();
}

}