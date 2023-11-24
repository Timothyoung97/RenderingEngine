#include "rendererGBuffer.h"

#include "utility.h"

namespace tre { 

RendererGBuffer::RendererGBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderInstanced.create(basePathWstr + L"shaders\\bin\\vertex_shader_instancedRendering.bin", _device);
	_pixelShaderInstanced.create(basePathWstr + L"shaders\\bin\\pixel_shader_instanced_gbuffer.bin", _device);
}

void RendererGBuffer::render(Graphics& graphics, Scene& scene, Camera& cam) {
	if (scene._culledOpaqueObjQ[scene.camViewIdx].empty()) return;

	const char* name = ToString(RENDER_MODE::INSTANCED_DEFERRED_OPAQUE_M);
	MICROPROFILE_SCOPE_CSTR(name);

	// View Projection Const Buffer
	ID3D11Buffer* constBufferCamViewProj = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::ViewProjectionStruct));
	{
		// Update const buffer and binding
		{
			tre::ViewProjectionStruct vpStruct = tre::CommonStructUtility::createViewProjectionStruct(cam.camViewProjection);
			tre::Buffer::updateConstBufferData(_context, constBufferCamViewProj, &vpStruct, (UINT)sizeof(tre::ViewProjectionStruct));
			_context->VSSetConstantBuffers(0u, 1u, &constBufferCamViewProj);
		}
	}

	// Batching
	graphics._instanceBuffer.updateBuffer(scene._culledOpaqueObjQ[scene.camViewIdx]);

	{
		_context->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		_context->VSSetShaderResources(0u, 1, graphics._instanceBuffer.pInstanceBufferSRV.GetAddressOf());

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		// unbind depth buffer as a shader resource, so that we can write to it
		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(graphics._instancedPixelShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(4, 1, graphics.nullSRV);

		_context->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->OMSetRenderTargets(2, graphics._gBuffer.rtvs, graphics._depthbuffer.pDepthStencilView.Get());
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	// Create an empty const buffer 
	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::BatchInfoStruct));
	
	PROFILE_GPU_SCOPED("G-Buffer Instanced Draw");
	for (int i = 0; i < graphics._instanceBuffer.instanceBatchQueue.size(); i++) {
		InstanceBatchInfo currBatchInfo = graphics._instanceBuffer.instanceBatchQueue[i];

		// update constant buffer for each instanced draw call
		{
			tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
			tre::Buffer::updateConstBufferData(_context, constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));

			_context->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);
		}

		// Update mesh vertex information
		_context->IASetVertexBuffers(0, 1, currBatchInfo.pBatchMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
		_context->IASetIndexBuffer(currBatchInfo.pBatchMesh->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Update texture information
		_context->PSSetShaderResources(0u, 1u, graphics.nullSRV);
		if (currBatchInfo.isWithTexture) {
			_context->PSSetShaderResources(0u, 1u, currBatchInfo.pBatchTexture->pShaderResView.GetAddressOf());
		}

		_context->PSSetShaderResources(1u, 1u, graphics.nullSRV);
		if (currBatchInfo.hasNormMap) {
			_context->PSSetShaderResources(1u, 1u, currBatchInfo.pBatchNormalMap->pShaderResView.GetAddressOf());
		}

		// Draw call
		_context->DrawIndexedInstanced(currBatchInfo.pBatchMesh->indexSize, currBatchInfo.quantity, 0u, 0u, 0u);
	}

	// clean up
	{
		graphics.bufferQueue.push_back(constBufferCamViewProj);
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}
}
}