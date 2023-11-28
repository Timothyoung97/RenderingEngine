#include "instanceBuffer.h"

#include "dxdebug.h"
#include "utility.h"

#include <set>

namespace tre {

void InstanceBuffer::createBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {

	_device = pDevice;
	_context = pContext;

	// Staging Buffer
	D3D11_BUFFER_DESC stagingInstancedBufferDesc;
	stagingInstancedBufferDesc.BindFlags = 0;
	stagingInstancedBufferDesc.Usage = D3D11_USAGE_STAGING;
	stagingInstancedBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	stagingInstancedBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	stagingInstancedBufferDesc.ByteWidth = static_cast<UINT>(sizeof(tre::InstanceInfo) * currMaxInstanceCount);
	stagingInstancedBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(tre::InstanceInfo));

	CHECK_DX_ERROR(_device->CreateBuffer(
		&stagingInstancedBufferDesc, nullptr, &pStagingInstanceBuffer
	));

	// Persistent Buffer
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


void InstanceBuffer::updateBuffer(const std::vector<std::pair<Object*, Mesh*>>& objQ) {

	MICROPROFILE_SCOPE_CSTR("Batching Instances");
	PROFILE_GPU_SCOPED("GPU Batching Instances");

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

	// If structured buffer on GPU has insufficient size
	if (currMaxInstanceCount < instanceInfoQ.size()) {
		currMaxInstanceCount = instanceInfoQ.size();
		pStagingInstanceBuffer.Reset();
		pInstanceBuffer.Reset();

		// Staging Buffer
		D3D11_BUFFER_DESC stagingInstancedBufferDesc;
		stagingInstancedBufferDesc.BindFlags = 0;
		stagingInstancedBufferDesc.Usage = D3D11_USAGE_STAGING;
		stagingInstancedBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		stagingInstancedBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		stagingInstancedBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * currMaxInstanceCount);
		stagingInstancedBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(tre::InstanceInfo));

		D3D11_SUBRESOURCE_DATA pInstanceBufferData = {};
		pInstanceBufferData.pSysMem = instanceInfoQ.data();

		CHECK_DX_ERROR(_device->CreateBuffer(
			&stagingInstancedBufferDesc, &pInstanceBufferData, &pStagingInstanceBuffer
		));

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
	} else {
		// Update Subresource Data
		D3D11_MAPPED_SUBRESOURCE previousInstancedData;
		ZeroMemory(&previousInstancedData, sizeof(D3D11_MAPPED_SUBRESOURCE));

		CHECK_DX_ERROR(_context->Map(pStagingInstanceBuffer.Get(), 0u, D3D11_MAP_WRITE, 0u, &previousInstancedData));

		memcpy(previousInstancedData.pData, instanceInfoQ.data(), static_cast<UINT>(sizeof(InstanceInfo) * instanceInfoQ.size()));

		_context->Unmap(pStagingInstanceBuffer.Get(), 0u);
	}

	_context->CopyResource(pInstanceBuffer.Get(), pStagingInstanceBuffer.Get());

	// SRV
	D3D11_BUFFER_SRV instanceBufferSRV;
	ZeroMemory(&instanceBufferSRV, sizeof(D3D11_BUFFER_SRV));
	instanceBufferSRV.FirstElement = 0u;
	instanceBufferSRV.NumElements = instanceInfoQ.size();

	D3D11_SHADER_RESOURCE_VIEW_DESC instanceBufferSRVDesc;
	instanceBufferSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	instanceBufferSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	instanceBufferSRVDesc.Buffer = instanceBufferSRV;

	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		pInstanceBuffer.Get(), &instanceBufferSRVDesc, pInstanceBufferSRV.GetAddressOf()
	));
}

void InstanceBuffer::updateBuffer(const std::vector<Object*>& objQ, Mesh* specifiedMesh) {

	if (objQ.empty()) return;

	MICROPROFILE_SCOPE_CSTR("Batching Instances Wireframe");
	PROFILE_GPU_SCOPED("GPU Batching Instances Wireframe");

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

	if (!quantity) return;

	InstanceBatchInfo singleBatch;
	singleBatch.batchStartIdx = 0;
	singleBatch.hasNormMap = 0;
	singleBatch.isWithTexture = 0;
	singleBatch.pBatchMesh = specifiedMesh;
	singleBatch.pBatchNormalMap = nullptr;
	singleBatch.pBatchTexture = nullptr;
	singleBatch.quantity = quantity;
	instanceBatchQueue.push_back(singleBatch);

	D3D11_BUFFER_DESC pInstanceBufferDesc;
	pInstanceBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	pInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	pInstanceBufferDesc.CPUAccessFlags = 0u;
	pInstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pInstanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * instanceInfoQ.size()); 
	pInstanceBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(InstanceInfo));

	D3D11_SUBRESOURCE_DATA pInstanceBufferData = {};
	pInstanceBufferData.pSysMem = instanceInfoQ.data();

	CHECK_DX_ERROR(_device->CreateBuffer(
		&pInstanceBufferDesc, &pInstanceBufferData, pInstanceBuffer.GetAddressOf()
	));

	// SRV
	D3D11_BUFFER_SRV instanceBufferSRV;
	instanceBufferSRV.NumElements = instanceInfoQ.size();
	instanceBufferSRV.FirstElement = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC instanceBufferSRVDesc;
	instanceBufferSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	instanceBufferSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	instanceBufferSRVDesc.Buffer = instanceBufferSRV;

	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		pInstanceBuffer.Get(), &instanceBufferSRVDesc, pInstanceBufferSRV.GetAddressOf()
	));

}

}