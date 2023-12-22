#include "rendererEnvironmentLighting.h"

#include "utility.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

RendererEnvironmentLighting::RendererEnvironmentLighting() {
	this->init();
}

void RendererEnvironmentLighting::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", pEngine->device->device.Get());
	_deferredShaderLightingEnv.create(basePathWstr + L"shaders\\bin\\pixel_shader_deferred_lighting_env.bin", pEngine->device->device.Get());
}

void RendererEnvironmentLighting::render(Graphics& graphics, const Scene& scene, const Camera& cam) {
	const char* name = ToString(RENDER_MODE::DEFERRED_OPAQUE_LIGHTING_ENV_M);
	MICROPROFILE_SCOPE_CSTR(name);
	//PROFILE_GPU_SCOPED("Deferred Environment Lighting Pass");

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Create Views
	ComPtr<ID3D11ShaderResourceView> ssaoBlurredTexture2dSRV;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC ssaoResultTexture2dSRVDesc;
		ssaoResultTexture2dSRVDesc.Format = DXGI_FORMAT_R8_UNORM;
		ssaoResultTexture2dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		ssaoResultTexture2dSRVDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._ssao.ssaoBlurredTexture2d.Get(), &ssaoResultTexture2dSRVDesc, ssaoBlurredTexture2dSRV.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> depthStencilShaderRescView;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._depthbuffer.pDepthStencilTexture.Get(), &shaderResourceViewDesc, depthStencilShaderRescView.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> shadowShaderRescView;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._depthbuffer.pShadowMapTexture.Get(), &shaderResourceViewDesc, shadowShaderRescView.GetAddressOf()
		));
	}

	// Context Configuration
	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(nullptr);
		contextD.Get()->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		contextD.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		contextD.Get()->PSSetShader(_deferredShaderLightingEnv.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		contextD.Get()->PSSetShaderResources(0, 1, graphics._gBuffer.pShaderResViewDeferredAlbedo.GetAddressOf()); // albedo
		contextD.Get()->PSSetShaderResources(1, 1, graphics._gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		contextD.Get()->PSSetShaderResources(3, 1, shadowShaderRescView.GetAddressOf()); // shadow
		contextD.Get()->PSSetShaderResources(4, 1, depthStencilShaderRescView.GetAddressOf()); //depth
		contextD.Get()->PSSetShaderResources(7, 1, ssaoBlurredTexture2dSRV.GetAddressOf()); // ssao
		contextD.Get()->PSSetSamplers(0, 1, graphics._sampler.pSamplerStateMinMagMipLinearWrap.GetAddressOf());
		contextD.Get()->PSSetSamplers(1, 1, graphics._sampler.pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf());

		contextD.Get()->OMSetRenderTargets(1, graphics._hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), nullptr);
		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
	}

	contextD.Get()->Draw(6, 0);

	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
	}

	{
		CHECK_DX_ERROR(contextD->FinishCommandList(
			false, &commandList
		));
	}
}
} 