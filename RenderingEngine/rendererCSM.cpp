#include "rendererCSM.h"
#include "microprofile.h"
#include "window.h"
#include "utility.h"

namespace tre {

RendererCSM::RendererCSM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderInstanced.create(basePathWstr + L"shaders\\bin\\vertex_shader_instancedRendering.bin", _device);
}

void RendererCSM::setCSMViewport(Graphics& graphics, int idx) {
	graphics._viewport.shadowViewport.TopLeftX = graphics._rasterizer.rectArr[idx].left;
	graphics._viewport.shadowViewport.TopLeftY = graphics._rasterizer.rectArr[idx].top;
	_context->RSSetViewports(1, &graphics._viewport.shadowViewport);
	_context->RSSetScissorRects(1, &graphics._rasterizer.rectArr[idx]);
}

void RendererCSM::drawInstanced(Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>>& objQ) {
	if (objQ.size() == 0) return;

	// Profiling
	const char* name = ToString(RENDER_MODE::INSTANCED_SHADOW_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("CSM Instanced Draw");

	// Update structured buffer for instanced draw call
	graphics._instanceBuffer.updateBuffer(objQ);

	// Configure context for CMS Drawc call
	{
		_context->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShaderInstanced.pShader.Get(), NULL, 0u);
		_context->VSSetShaderResources(0u, 1, graphics._instanceBuffer.pInstanceBufferSRV.GetAddressOf());

		// use setShadowBufferDrawSection to select draw section
		_context->RSSetState(graphics._rasterizer.pShadowRasterizerState.Get());

		// unbind shadow buffer as a resource, so that we can write to it
		_context->PSSetShader(nullptr, NULL, 0u);
		_context->PSSetShaderResources(3, 1, graphics.nullSRV);

		_context->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
		_context->OMSetRenderTargets(0, nullptr, graphics._depthbuffer.pShadowDepthStencilView.Get());
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	// Create an empty const buffer 
	ID3D11Buffer* constBufferBatchInfo = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::BatchInfoStruct));
	_context->VSSetConstantBuffers(1u, 1u, &constBufferBatchInfo);

	for (int i = 0; i < graphics._instanceBuffer.instanceBatchQueue.size(); i++) {
		InstanceBatchInfo currBatchInfo = graphics._instanceBuffer.instanceBatchQueue[i];

		// update constant buffer for each instanced draw call
		{
			tre::BatchInfoStruct bInfo = tre::CommonStructUtility::createBatchInfoStruct(currBatchInfo.batchStartIdx);
			tre::Buffer::updateConstBufferData(_context, constBufferBatchInfo, &bInfo, (UINT)sizeof(tre::BatchInfoStruct));
		}

		// Update mesh vertex information
		{
			_context->IASetVertexBuffers(0, 1, currBatchInfo.pBatchMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
			_context->IASetIndexBuffer(currBatchInfo.pBatchMesh->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		}

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

	// Pushing used const buffer to queue for cleaning
	{
		graphics.bufferQueue.push_back(constBufferBatchInfo);
	}
}

void RendererCSM::render(Graphics& graphics, Scene& scene, const Camera& cam) {

	ID3D11Buffer* constBufferCSMViewProj = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::ViewProjectionStruct));
	{
		PROFILE_GPU_SCOPED("CSM Quad Draw");

		// viewIdx: 0 -> reserved for camera
		for (int viewIdx = 1; viewIdx < 5; viewIdx++) {

			{	// Culling based on pointlight's view projection matrix
				MICROPROFILE_SCOPE_CSTR("CSM Quad Obj Culling");

				setCSMViewport(graphics, viewIdx - 1);

				// Const Buffer 
				{
					// Create struct info and submit data to const buffer
					tre::ViewProjectionStruct csmViewProjStruct = tre::CommonStructUtility::createViewProjectionStruct(scene.viewProjs[viewIdx]);
					tre::Buffer::updateConstBufferData(_context, constBufferCSMViewProj, &csmViewProjStruct, (UINT)sizeof(tre::ViewProjectionStruct));

					// Binding 
					_context->VSSetConstantBuffers(0u, 1u, &constBufferCSMViewProj);
				}

				graphics.stats.shadowCascadeOpaqueObjs[viewIdx] = scene._culledOpaqueObjQ.size();
			}

			// draw shadow only for opaque objects
			//renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::SHADOW_M); // non instanced
			drawInstanced(graphics, scene._culledOpaqueObjQ[viewIdx]); // instanced
		}
	}

	// Pushing used const buffer to queue for cleaning
	{
		graphics.bufferQueue.push_back(constBufferCSMViewProj);
	}
}

}