#include "instanceBuffer.h"

#include "dxdebug.h"

namespace tre {

void InstanceBuffer::createBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {

	_device = pDevice;
	_context = pContext;
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

	D3D11_BUFFER_DESC pInstanceBufferDesc;
	pInstanceBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	pInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	pInstanceBufferDesc.CPUAccessFlags = 0u;
	pInstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pInstanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * instanceInfoQ.size()); //hardcoded initial size
	pInstanceBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(InstanceInfo));
	
	D3D11_SUBRESOURCE_DATA pInstanceBufferData = {};
	pInstanceBufferData.pSysMem = instanceInfoQ.data();

	ID3D11Buffer* newInstanceBuffer;
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

void InstanceBuffer::updateBuffer(const std::vector<std::pair<Object*, Mesh*>>& objQ, Mesh* specifiedMesh) {
	std::vector<InstanceInfo> instanceInfoQ;
	instanceBatchQueue.clear();

	InstanceBatchInfo singleBatch;
	singleBatch.batchStartIdx = 0;
	singleBatch.hasNormMap = 0;
	singleBatch.isWithTexture = 0;
	singleBatch.pBatchMesh = specifiedMesh;
	singleBatch.pBatchNormalMap = nullptr;
	singleBatch.pBatchTexture = nullptr;
	singleBatch.quantity = objQ.size();
	instanceBatchQueue.push_back(singleBatch);

	for (int i = 0; i < objQ.size(); i++) {
		// Each instance's information
		Object* pObj = objQ[i].first;
		for (int j = 0; j < pObj->_boundingVolumeTransformation.size(); j++) {
			// Push each instance's information to the vector
			InstanceInfo newInstInfo = this->createInstanceInfo(pObj->_boundingVolumeTransformation[j], pObj->_boundingVolumeColor[j], 0u, 0u);
			instanceInfoQ.push_back(newInstInfo);
		}
	}

	D3D11_BUFFER_DESC pInstanceBufferDesc;
	pInstanceBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	pInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	pInstanceBufferDesc.CPUAccessFlags = 0u;
	pInstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pInstanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * instanceInfoQ.size()); //hardcoded initial size
	pInstanceBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(InstanceInfo));

	D3D11_SUBRESOURCE_DATA pInstanceBufferData = {};
	pInstanceBufferData.pSysMem = instanceInfoQ.data();

	ID3D11Buffer* newInstanceBuffer;
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