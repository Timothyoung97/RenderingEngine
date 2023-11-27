#include "rendererLocalLighting.h"

#include "utility.h"

namespace tre {

RendererLocalLighting::RendererLocalLighting(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", _device);
	_deferredShaderLightingLocal.create(basePathWstr + L"shaders\\bin\\pixel_shader_deferred_lighting_local.bin", _device);
}

void RendererLocalLighting::render(Graphics& graphics, const Scene& scene, const Camera& cam) {
	if (scene._wireframeObjQ.size() == 0) return;

	const char* name = ToString(DEFERRED_LIGHTING_LOCAL_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Deferred Local Light Draw");

	{
		_context->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get()); // by default: render only front face

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_deferredShaderLightingLocal.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(0, 1, graphics._gBuffer.pShaderResViewDeferredAlbedo.GetAddressOf()); // albedo
		_context->PSSetShaderResources(1, 1, graphics._gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		_context->PSSetShaderResources(2, 1, scene.lightResc.pLightShaderRescView.GetAddressOf());			// point light info
		_context->CopyResource(graphics._depthbuffer.pDepthStencilReadOnlyTexture.Get(), graphics._depthbuffer.pDepthStencilTexture.Get());
		_context->PSSetShaderResources(4, 1, graphics._depthbuffer.pDepthStencilReadOnlyShaderRescView.GetAddressOf()); //depth

		_context->OMSetBlendState(graphics._blendstate.lighting.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		_context->OMSetRenderTargets(1, graphics._hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), graphics._depthbuffer.pDepthStencilView.Get()); // draw to HDR floating point buffer
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	//Set vertex buffer
	_context->IASetVertexBuffers(0, 1, scene._wireframeObjQ[0]->pObjMeshes[0]->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

	//Set index buffer
	_context->IASetIndexBuffer(scene._wireframeObjQ[0]->pObjMeshes[0]->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Create empty const buffer and pre bind the constant buffer
	ID3D11Buffer* constBufferModelInfo = tre::Buffer::createConstBuffer(_device, sizeof(tre::ModelInfoStruct));
	_context->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
	_context->PSSetConstantBuffers(1u, 1u, &constBufferModelInfo);

	ID3D11Buffer* constBufferPtLightInfo = tre::Buffer::createConstBuffer(_device, sizeof(tre::PointLightInfoStruct));
	_context->PSSetConstantBuffers(2u, 1u, &constBufferPtLightInfo);

	for (int i = 0; i < scene._wireframeObjQ.size(); i++) {

		// Submit each object's data to const buffer
		{
			tre::ModelInfoStruct modelInfoStruct = tre::CommonStructUtility::createModelInfoStruct(scene._wireframeObjQ[i]->_transformationFinal, scene._wireframeObjQ[i]->pObjMeshes[0]->pMaterial->baseColor, 0u, 0u);
			tre::Buffer::updateConstBufferData(_context, constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
		}

		// Submit point light's idx to const buffer
		{
			tre::PointLightInfoStruct ptLightInfoStruct = tre::CommonStructUtility::createPointLightInfoStruct(i);
			tre::Buffer::updateConstBufferData(_context, constBufferPtLightInfo, &ptLightInfoStruct, sizeof(tre::PointLightInfoStruct));
		}

		float distFromObjToCam = tre::Maths::distBetweentObjToCam(scene._wireframeObjQ[i]->objPos, cam.camPositionV);

		// if the camera is inside the light sphere
		if (distFromObjToCam < scene._wireframeObjQ[i]->objScale.x) {
			_context->RSSetState(graphics._rasterizer.pRasterizerStateFCW.Get()); // render only back face
			_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithoutDepthT.Get(), 0); // all depth test pass to render the sphere
		}

		_context->DrawIndexed(scene._wireframeObjQ[i]->pObjMeshes[0]->indexSize, 0, 0);
	}

	// clean up
	{
		graphics.bufferQueue.push_back(constBufferModelInfo);
		graphics.bufferQueue.push_back(constBufferPtLightInfo);
	}
}

}