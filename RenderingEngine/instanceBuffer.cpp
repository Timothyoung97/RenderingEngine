#include "instanceBuffer.h"

#include "dxdebug.h"

namespace tre {

void InstanceBuffer::createBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {

	_device = pDevice;
	_context = pContext;

	D3D11_BUFFER_DESC pInstanceBufferDesc;
	pInstanceBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	pInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	pInstanceBufferDesc.CPUAccessFlags = 0u;
	pInstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pInstanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * lastBufferSize); //hardcoded initial size
	pInstanceBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(InstanceInfo));

	CHECK_DX_ERROR(_device->CreateBuffer(
		&pInstanceBufferDesc, NULL, pInstanceBuffer.GetAddressOf()
	));;
}

void InstanceBuffer::updateBuffer(const std::vector<InstanceInfo>& infoQ) {
	
	if (infoQ.size() > lastBufferSize) {
		D3D11_BUFFER_DESC pInstanceBufferDesc;
		pInstanceBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		pInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		pInstanceBufferDesc.CPUAccessFlags = 0u;
		pInstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		pInstanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * infoQ.size()); //hardcoded initial size
		pInstanceBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(InstanceInfo));
		
		ID3D11Buffer* newInstanceBuffer;
		CHECK_DX_ERROR(_device->CreateBuffer(
			&pInstanceBufferDesc, NULL, &newInstanceBuffer
		));


	}
	
	
};

}