#include "renderer.h"
#include "mesh.h"
#include "device.h"
#include "dxdebug.h"

namespace tre {

Renderer::Renderer() {}

void Renderer::draw(ID3D11Device* device, ID3D11DeviceContext* context, tre::ConstantBuffer& cb, const std::vector<Object>& objQ) {

	for (int i = 0; i < objQ.size(); i++) {

		const tre::Object& currObj = objQ[i];

		//Set vertex buffer
		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, currObj.pObjMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		context->IASetIndexBuffer(currObj.pObjMesh->pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//set shader resc view and sampler
		context->PSSetShaderResources(0, 1, currObj.pObjTexture->pShaderResView.GetAddressOf());

		//Config const buffer
		cb.constBufferRescModel.transformationLocal = XMMatrixMultiply(
			XMMatrixScaling(currObj.objScale.x, currObj.objScale.y, currObj.objScale.z),
			XMMatrixMultiply(
				XMMatrixRotationRollPitchYaw(XMConvertToRadians(currObj.objRotation.x), XMConvertToRadians(currObj.objRotation.x), XMConvertToRadians(currObj.objRotation.x)),
				XMMatrixTranslation(currObj.objPos.x, currObj.objPos.y, currObj.objPos.z)
			)
		);
		
		XMFLOAT3X3 normalFloat3x3;
		XMStoreFloat3x3(&normalFloat3x3, cb.constBufferRescModel.transformationLocal);
		XMMATRIX normalMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat3x3(&normalFloat3x3)));
		cb.constBufferRescModel.normalMatrix = normalMatrix;

		cb.constBufferRescModel.isWithTexture = currObj.isObjWithTexture;
		cb.constBufferRescModel.color = currObj.objColor;

		//map to data to subresouce
		cb.csd.pSysMem = &cb.constBufferRescModel;

		CHECK_DX_ERROR(device->CreateBuffer(
			&cb.constantBufferDescModel, &cb.csd, cb.pConstBuffer.GetAddressOf()
		));

		context->VSSetConstantBuffers(1u, 1u, cb.pConstBuffer.GetAddressOf());
		context->PSSetConstantBuffers(1u, 1u, cb.pConstBuffer.GetAddressOf());

		context->DrawIndexed(currObj.pObjMesh->indexSize, 0, 0);
	}
}
}