#include "rendererTransparency.h"

#include "utility.h"

namespace tre {

RendererTransparency::RendererTransparency(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", _device);
	_forwardShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_forward.bin", _device);
}

void RendererTransparency::render(Graphics& graphics, const Scene& scene, const Camera& cam) {
	if (scene._culledTransparentObjQ.size() == 0) return;

	const char* name = ToString(RENDER_MODE::TRANSPARENT_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Forward Tranparent Draw");

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(_context, constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Create empty const buffer and pre bind the constant buffer
	ID3D11Buffer* constBufferModelInfo = tre::Buffer::createConstBuffer(_device, sizeof(tre::ModelInfoStruct));

	// Context Configuration
	{
		_context->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);
		_context->VSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		_context->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		_context->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
		_context->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		_context->PSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
		_context->PSSetShaderResources(3, 1, graphics._depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		_context->PSSetShaderResources(4, 1, graphics.nullSRV);

		_context->OMSetBlendState(graphics._blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0);
		_context->OMSetRenderTargets(1, graphics._hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), graphics._depthbuffer.pDepthStencilView.Get());
	}

	for (int i = 0; i < scene._culledTransparentObjQ.size(); i++) {

		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;

		//Set vertex buffer
		_context->IASetVertexBuffers(0, 1, scene._culledTransparentObjQ[i].second->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		_context->IASetIndexBuffer(scene._culledTransparentObjQ[i].second->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//set shader resc view and sampler
		bool hasTexture = 0;
		bool hasNormal = 0;
		if (scene._culledTransparentObjQ[i].second->pMaterial->objTexture != nullptr) {
			_context->PSSetShaderResources(0, 1, scene._culledTransparentObjQ[i].second->pMaterial->objTexture->pShaderResView.GetAddressOf());
			hasTexture = 1;
		}

		// set normal map
		if (scene._culledTransparentObjQ[i].second->pMaterial->objNormalMap != nullptr) {
			_context->PSSetShaderResources(1, 1, scene._culledTransparentObjQ[i].second->pMaterial->objNormalMap->pShaderResView.GetAddressOf());
			hasNormal = 1;
		}

		// Submit each object's data to const buffer
		{
			tre::ModelInfoStruct modelInfoStruct = tre::CommonStructUtility::createModelInfoStruct(scene._culledTransparentObjQ[i].first->_transformationFinal, scene._culledTransparentObjQ[i].second->pMaterial->baseColor, hasTexture, hasNormal);
			tre::Buffer::updateConstBufferData(_context, constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
		}

		_context->DrawIndexed(scene._culledTransparentObjQ[i].second->indexSize, 0, 0);
	}

	// clean up
	{
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
		graphics.bufferQueue.push_back(constBufferModelInfo);
	}
}

}