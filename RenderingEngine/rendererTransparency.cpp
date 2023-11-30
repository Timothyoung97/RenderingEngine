#include "rendererTransparency.h"

#include "utility.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

RendererTransparency::RendererTransparency() {
	this->init();
}

void RendererTransparency::init() {
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", pEngine->device->device.Get());
	_forwardShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_forward.bin", pEngine->device->device.Get());
}

void RendererTransparency::render(Graphics& graphics, const Scene& scene, const Camera& cam) {
	if (scene._culledTransparentObjQ.size() == 0) return;

	const char* name = ToString(RENDER_MODE::TRANSPARENT_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Forward Tranparent Draw");

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Create empty const buffer and pre bind the constant buffer
	ID3D11Buffer* constBufferModelInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), sizeof(tre::ModelInfoStruct));

	// Context Configuration
	{
		pEngine->device->context.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		pEngine->device->context.Get()->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);
		pEngine->device->context.Get()->VSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		pEngine->device->context.Get()->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);

		pEngine->device->context.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		pEngine->device->context.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		pEngine->device->context.Get()->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
		pEngine->device->context.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		pEngine->device->context.Get()->PSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
		pEngine->device->context.Get()->PSSetShaderResources(3, 1, graphics._depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		pEngine->device->context.Get()->PSSetShaderResources(4, 1, graphics.nullSRV);

		pEngine->device->context.Get()->OMSetBlendState(graphics._blendstate.transparency.Get(), NULL, 0xffffffff);
		pEngine->device->context.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0);
		pEngine->device->context.Get()->OMSetRenderTargets(1, graphics._hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), graphics._depthbuffer.pDepthStencilView.Get());
	}

	for (int i = 0; i < scene._culledTransparentObjQ.size(); i++) {

		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;

		//Set vertex buffer
		pEngine->device->context.Get()->IASetVertexBuffers(0, 1, scene._culledTransparentObjQ[i].second->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		pEngine->device->context.Get()->IASetIndexBuffer(scene._culledTransparentObjQ[i].second->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//set shader resc view and sampler
		bool hasTexture = 0;
		bool hasNormal = 0;
		if (scene._culledTransparentObjQ[i].second->pMaterial->objTexture != nullptr) {
			pEngine->device->context.Get()->PSSetShaderResources(0, 1, scene._culledTransparentObjQ[i].second->pMaterial->objTexture->pShaderResView.GetAddressOf());
			hasTexture = 1;
		}

		// set normal map
		if (scene._culledTransparentObjQ[i].second->pMaterial->objNormalMap != nullptr) {
			pEngine->device->context.Get()->PSSetShaderResources(1, 1, scene._culledTransparentObjQ[i].second->pMaterial->objNormalMap->pShaderResView.GetAddressOf());
			hasNormal = 1;
		}

		// Submit each object's data to const buffer
		{
			tre::ModelInfoStruct modelInfoStruct = tre::CommonStructUtility::createModelInfoStruct(scene._culledTransparentObjQ[i].first->_transformationFinal, scene._culledTransparentObjQ[i].second->pMaterial->baseColor, hasTexture, hasNormal);
			tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
		}

		pEngine->device->context.Get()->DrawIndexed(scene._culledTransparentObjQ[i].second->indexSize, 0, 0);
	}

	// clean up
	{
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
		graphics.bufferQueue.push_back(constBufferModelInfo);
	}
}

}