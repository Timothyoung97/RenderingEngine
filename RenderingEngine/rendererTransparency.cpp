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
	//PROFILE_GPU_SCOPED("Forward Tranparent Draw");

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Create empty const buffer and pre bind the constant buffer
	ID3D11Buffer* constBufferModelInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), sizeof(tre::ModelInfoStruct));

	// Context Configuration
	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		contextD.Get()->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);
		contextD.Get()->VSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		contextD.Get()->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		contextD.Get()->PSSetShader(_forwardShader.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		contextD.Get()->PSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
		contextD.Get()->PSSetSamplers(0, 1, graphics._sampler.pSamplerStateMinMagMipLinearWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(1, 1, graphics._sampler.pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf());
		contextD.Get()->PSSetShaderResources(3, 1, graphics._depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		contextD.Get()->PSSetShaderResources(4, 1, graphics.nullSRV);

		contextD.Get()->OMSetBlendState(graphics._blendstate.transparency.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0);
		contextD.Get()->OMSetRenderTargets(1, graphics._hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), graphics._depthbuffer.pDepthStencilView.Get());
	}

	for (int i = 0; i < scene._culledTransparentObjQ.size(); i++) {

		UINT vertexStride = sizeof(Vertex);
		UINT offset = 0;

		//Set vertex buffer
		contextD.Get()->IASetVertexBuffers(0, 1, scene._culledTransparentObjQ[i].second->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

		//Set index buffer
		contextD.Get()->IASetIndexBuffer(scene._culledTransparentObjQ[i].second->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		//set shader resc view and sampler
		bool hasTexture = 0;
		bool hasNormal = 0;
		if (scene._culledTransparentObjQ[i].second->pMaterial->objTexture != nullptr) {
			contextD.Get()->PSSetShaderResources(0, 1, scene._culledTransparentObjQ[i].second->pMaterial->objTexture->pShaderResView.GetAddressOf());
			hasTexture = 1;
		}

		// set normal map
		if (scene._culledTransparentObjQ[i].second->pMaterial->objNormalMap != nullptr) {
			contextD.Get()->PSSetShaderResources(1, 1, scene._culledTransparentObjQ[i].second->pMaterial->objNormalMap->pShaderResView.GetAddressOf());
			hasNormal = 1;
		}

		// Submit each object's data to const buffer
		{
			tre::ModelInfoStruct modelInfoStruct = tre::CommonStructUtility::createModelInfoStruct(scene._culledTransparentObjQ[i].first->_transformationFinal, scene._culledTransparentObjQ[i].second->pMaterial->baseColor, hasTexture, hasNormal);
			tre::Buffer::updateConstBufferData(contextD.Get(), constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
		}

		contextD.Get()->DrawIndexed(scene._culledTransparentObjQ[i].second->indexSize, 0, 0);
	}

	// clean up
	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
		graphics.bufferQueue.push_back(constBufferModelInfo);
	}

	{	
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}
}

}