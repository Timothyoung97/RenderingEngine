#include "rendererLocalLighting.h"

#include "utility.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

RendererLocalLighting::RendererLocalLighting() {
	this->init();
}

void RendererLocalLighting::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", pEngine->device->device.Get());
	_deferredShaderLightingLocal.create(basePathWstr + L"shaders\\bin\\pixel_shader_deferred_lighting_local.bin", pEngine->device->device.Get());
}

void RendererLocalLighting::render(Graphics& graphics, const Scene& scene, const Camera& cam) {
	if (scene._wireframeObjQ.size() == 0) return;

	const char* name = ToString(DEFERRED_LIGHTING_LOCAL_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Deferred Local Light Draw");

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Create empty const buffer and pre bind the constant buffer
	ID3D11Buffer* constBufferModelInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), sizeof(tre::ModelInfoStruct));
	ID3D11Buffer* constBufferPtLightInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), sizeof(tre::PointLightInfoStruct));

	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		contextD.Get()->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);
		contextD.Get()->VSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		contextD.Get()->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get()); // by default: render only front face

		contextD.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		contextD.Get()->PSSetShader(_deferredShaderLightingLocal.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		contextD.Get()->PSSetConstantBuffers(2u, 1u, &constBufferPtLightInfo);
		contextD.Get()->PSSetShaderResources(0, 1, graphics._gBuffer.pShaderResViewDeferredAlbedo.GetAddressOf()); // albedo
		contextD.Get()->PSSetShaderResources(1, 1, graphics._gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		contextD.Get()->PSSetShaderResources(2, 1, scene.lightResc.pLightShaderRescView.GetAddressOf());			// point light info
		contextD.Get()->CopyResource(graphics._depthbuffer.pDepthStencilReadOnlyTexture.Get(), graphics._depthbuffer.pDepthStencilTexture.Get());
		contextD.Get()->PSSetShaderResources(4, 1, graphics._depthbuffer.pDepthStencilReadOnlyShaderRescView.GetAddressOf()); //depth

		contextD.Get()->OMSetBlendState(graphics._blendstate.lighting.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		contextD.Get()->OMSetRenderTargets(1, graphics._hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), graphics._depthbuffer.pDepthStencilView.Get()); // draw to HDR floating point buffer
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	//Set vertex buffer
	contextD.Get()->IASetVertexBuffers(0, 1, scene._wireframeObjQ[0]->pObjMeshes[0]->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

	//Set index buffer
	contextD.Get()->IASetIndexBuffer(scene._wireframeObjQ[0]->pObjMeshes[0]->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	for (int i = 0; i < scene._wireframeObjQ.size(); i++) {

		// Submit each object's data to const buffer
		{
			tre::ModelInfoStruct modelInfoStruct = tre::CommonStructUtility::createModelInfoStruct(scene._wireframeObjQ[i]->_transformationFinal, scene._wireframeObjQ[i]->pObjMeshes[0]->pMaterial->baseColor, 0u, 0u);
			tre::Buffer::updateConstBufferData(contextD.Get(), constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
		}

		// Submit point light's idx to const buffer
		{
			tre::PointLightInfoStruct ptLightInfoStruct = tre::CommonStructUtility::createPointLightInfoStruct(i);
			tre::Buffer::updateConstBufferData(contextD.Get(), constBufferPtLightInfo, &ptLightInfoStruct, sizeof(tre::PointLightInfoStruct));
		}

		float distFromObjToCam = tre::Maths::distBetweentObjToCam(scene._wireframeObjQ[i]->objPos, cam.camPositionV);

		// if the camera is inside the light sphere
		if (distFromObjToCam < scene._wireframeObjQ[i]->objScale.x) {
			contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCW.Get()); // render only back face
			contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithoutDepthT.Get(), 0); // all depth test pass to render the sphere
		}

		contextD.Get()->DrawIndexed(scene._wireframeObjQ[i]->pObjMeshes[0]->indexSize, 0, 0);
	}

	// clean up
	{
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
		graphics.bufferQueue.push_back(constBufferModelInfo);
		graphics.bufferQueue.push_back(constBufferPtLightInfo);
	}

	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}
}

}