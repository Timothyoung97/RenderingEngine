#include "rendererCSM.h"
#include "microprofile.h"
#include "window.h"
#include "utility.h"

namespace tre {

RendererCSM::RendererCSM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderInstanced.create(basePathWstr + L"shaders\\bin\\vertex_shader_instancedRendering.bin", _device);
}

void RendererCSM::generateCSMViewProj(const Graphics& graphics, Scene& scene, const Camera& cam) {
	std::vector<XMMATRIX> csmViewProjs;

	for (int i = 0; i < 4; i++) { // for 4 quads

		MICROPROFILE_SCOPE_CSTR("Build CSM View Projection Matrix");

		// projection matrix of camera with specific near and far plane
		XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, graphics.setting.csmPlaneIntervals[i], graphics.setting.csmPlaneIntervals[i + 1]);

		std::vector<XMVECTOR> corners = tre::Maths::getFrustumCornersWorldSpace(XMMatrixMultiply(cam.camView, projMatrix));

		XMVECTOR center = tre::Maths::getAverageVector(corners);

		XMMATRIX lightView = XMMatrixLookAtLH(center + XMVECTOR{ scene.dirlight.direction.x, scene.dirlight.direction.y, scene.dirlight.direction.z }, center, XMVECTOR{ .0f, 1.f, .0f });

		XMMATRIX lightOrthoProj = tre::Maths::createOrthoMatrixFromFrustumCorners(10.f, corners, lightView);

		XMMATRIX lightViewProj = XMMatrixMultiply(lightView, lightOrthoProj);

		csmViewProjs.push_back(lightViewProj);
	}

	scene.csmViewProjs = csmViewProjs;
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

	generateCSMViewProj(graphics, scene, cam);

	ID3D11Buffer* constBufferCSMViewProj = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::ViewProjectionStruct));
	{
		PROFILE_GPU_SCOPED("CSM Quad Draw");

		for (int i = 0; i < 4; i++) {

			{	// Culling based on pointlight's view projection matrix
				MICROPROFILE_SCOPE_CSTR("CSM Quad Obj Culling");

				setCSMViewport(graphics, i);

				// Const Buffer 
				{
					// Create struct info and submit data to const buffer
					tre::ViewProjectionStruct csmViewProjStruct = tre::CommonStructUtility::createViewProjectionStruct(scene.csmViewProjs[i]);
					tre::Buffer::updateConstBufferData(_context, constBufferCSMViewProj, &csmViewProjStruct, (UINT)sizeof(tre::ViewProjectionStruct));

					// Binding 
					_context->VSSetConstantBuffers(0u, 1u, &constBufferCSMViewProj);
				}

				tre::Frustum lightFrustum = tre::Maths::createFrustumFromViewProjectionMatrix(scene.csmViewProjs[i]);

				scene.cullObject(lightFrustum, graphics.setting.typeOfBound);
				{
					MICROPROFILE_SCOPE_CSTR("Grouping instances (Opaque only)");
					scene.updateCulledOpaqueQ();
				}

				graphics.stats.shadowCascadeOpaqueObjs[i] = scene._culledOpaqueObjQ.size();
			}

			// draw shadow only for opaque objects
			//renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::SHADOW_M); // non instanced
			drawInstanced(graphics, scene._culledOpaqueObjQ); // instanced
		}
	}

	// Pushing used const buffer to queue for cleaning
	{
		graphics.bufferQueue.push_back(constBufferCSMViewProj);
	}
}

}