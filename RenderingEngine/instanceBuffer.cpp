#include "instanceBuffer.h"

#include "dxdebug.h"

namespace tre {

void InstanceBuffer::createBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {

	_device = pDevice;
	_context = pContext;
}

void InstanceBuffer::updateBuffer(const std::vector<InstanceInfo>& infoQ) {
	
	D3D11_BUFFER_DESC pInstanceBufferDesc;
	pInstanceBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	pInstanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	pInstanceBufferDesc.CPUAccessFlags = 0u;
	pInstanceBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pInstanceBufferDesc.ByteWidth = static_cast<UINT>(sizeof(InstanceInfo) * infoQ.size()); //hardcoded initial size
	pInstanceBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(InstanceInfo));
	
	D3D11_SUBRESOURCE_DATA pInstanceBufferData = {};
	pInstanceBufferData.pSysMem = infoQ.data();

	ID3D11Buffer* newInstanceBuffer;
	CHECK_DX_ERROR(_device->CreateBuffer(
		&pInstanceBufferDesc, &pInstanceBufferData, pInstanceBuffer.GetAddressOf()
	));

	// SRV
	D3D11_BUFFER_SRV instanceBufferSRV;
	instanceBufferSRV.NumElements = infoQ.size();
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