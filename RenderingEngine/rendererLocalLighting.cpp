#include "rendererLocalLighting.h"

namespace tre {

RendererLocalLighting::RendererLocalLighting() {
	this->init();
}

void RendererLocalLighting::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", pEngine->device->device.Get());
	_deferredShaderLightingLocal.create(basePathWstr + L"shaders\\bin\\pixel_shader_deferred_lighting_local.bin", pEngine->device->device.Get());
}

void RendererLocalLighting::render(Graphics& graphics, const Scene& scene, const Camera& cam, MicroProfiler& profiler) {
	if (scene._wireframeObjQ.size() == 0) return;

	MICROPROFILE_SCOPE_CSTR("Point Light Section");
	profiler.graphicsGpuThreadLogStatus[5] = 1;
	MICROPROFILE_CONDITIONAL(MicroProfileThreadLogGpu * pMicroProfileLog = profiler.graphicsGpuThreadLog[5]);
	MICROPROFILE_GPU_BEGIN(contextD.Get(), pMicroProfileLog);

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

	// Create Views
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateDepthStencilView(
			graphics._depthbuffer.pDepthStencilTexture.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> depthStencilReadOnlyShaderRescView;
	{
		contextD.Get()->CopyResource(graphics._depthbuffer.pDepthStencilReadOnlyTexture.Get(), graphics._depthbuffer.pDepthStencilTexture.Get());

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._depthbuffer.pDepthStencilReadOnlyTexture.Get(), &shaderResourceViewDesc, depthStencilReadOnlyShaderRescView.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> lightShaderRescView;
	{
		// update GPU on buffer
		D3D11_BUFFER_SRV lightBufferSRV;
		lightBufferSRV.NumElements = scene.lightResc.numOfLights;
		lightBufferSRV.FirstElement = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC lightShaderResc;
		lightShaderResc.Format = DXGI_FORMAT_UNKNOWN;
		lightShaderResc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		lightShaderResc.Buffer = lightBufferSRV;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			scene.lightResc.pLightBufferGPU.Get(), &lightShaderResc, lightShaderRescView.GetAddressOf()
		));
	}

	ComPtr<ID3D11RenderTargetView> hdrRTV;
	{
		D3D11_RENDER_TARGET_VIEW_DESC hdrRTVDesc;
		ZeroMemory(&hdrRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		hdrRTVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		hdrRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		hdrRTVDesc.Texture2D.MipSlice = 0;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateRenderTargetView(
			graphics._hdrBuffer.pHdrBufferTexture.Get(), &hdrRTVDesc, hdrRTV.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> deferredAlbedoSRV;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
		shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._gBuffer.pGBufferTextureAlbedo.Get(), &shaderResViewDesc, deferredAlbedoSRV.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> deferredNormalSRV;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
		shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._gBuffer.pGBufferTextureNormal.Get(), &shaderResViewDesc, deferredNormalSRV.GetAddressOf()
		));
	}

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
		contextD.Get()->PSSetShaderResources(0, 1, deferredAlbedoSRV.GetAddressOf()); // albedo
		contextD.Get()->PSSetShaderResources(1, 1, deferredNormalSRV.GetAddressOf()); // normal
		contextD.Get()->PSSetShaderResources(2, 1, lightShaderRescView.GetAddressOf());			// point light info
		contextD.Get()->PSSetShaderResources(4, 1, depthStencilReadOnlyShaderRescView.GetAddressOf()); //depth

		contextD.Get()->OMSetBlendState(graphics._blendstate.lighting.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		contextD.Get()->OMSetRenderTargets(1, hdrRTV.GetAddressOf(), depthStencilView.Get()); // draw to HDR floating point buffer
	}

	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;

	//Set vertex buffer
	contextD.Get()->IASetVertexBuffers(0, 1, scene._wireframeObjQ[0]->pObjMeshes[0]->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

	//Set index buffer
	contextD.Get()->IASetIndexBuffer(scene._wireframeObjQ[0]->pObjMeshes[0]->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	{
		MICROPROFILE_SECTIONGPUI_L(pMicroProfileLog, "Point Light Section", tre::Utility::getRandomInt(INT_MAX));
		MICROPROFILE_SCOPEGPU_TOKEN_L(pMicroProfileLog, profiler.graphicsTokenGpuFrameIndex[5]);
		MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "Point Light: Draw", tre::Utility::getRandomInt(INT_MAX));

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
	}

	// clean up
	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
		graphics.bufferQueue.push_back(constBufferModelInfo);
		graphics.bufferQueue.push_back(constBufferPtLightInfo);
	}

	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}

	profiler.graphicsMicroProfile[5] = MICROPROFILE_GPU_END(pMicroProfileLog);
}

}