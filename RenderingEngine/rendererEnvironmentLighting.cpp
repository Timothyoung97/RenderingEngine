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
	PROFILE_GPU_SCOPED("Deferred Environment Lighting Pass");

	// set const buffer for global info
	ID3D11Buffer* constBufferGlobalInfo = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::GlobalInfoStruct globalInfoStruct = tre::CommonStructUtility::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, scene.viewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
		tre::Buffer::updateConstBufferData(pEngine->device->contextI.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));
	}

	// Context Configuration
	{
		pEngine->device->contextI.Get()->IASetInputLayout(nullptr);
		pEngine->device->contextI.Get()->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		pEngine->device->contextI.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		pEngine->device->contextI.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		pEngine->device->contextI.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		pEngine->device->contextI.Get()->PSSetShader(_deferredShaderLightingEnv.pShader.Get(), NULL, 0u);
		pEngine->device->contextI.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		pEngine->device->contextI.Get()->PSSetShaderResources(0, 1, graphics._gBuffer.pShaderResViewDeferredAlbedo.GetAddressOf()); // albedo
		pEngine->device->contextI.Get()->PSSetShaderResources(1, 1, graphics._gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		pEngine->device->contextI.Get()->PSSetShaderResources(3, 1, graphics._depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		pEngine->device->contextI.Get()->PSSetShaderResources(4, 1, graphics._depthbuffer.pDepthStencilShaderRescView.GetAddressOf()); //depth
		pEngine->device->contextI.Get()->PSSetShaderResources(7, 1, graphics._ssao.ssaoBlurredTexture2dSRV.GetAddressOf()); // ssao

		pEngine->device->contextI.Get()->OMSetRenderTargets(1, graphics._hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), nullptr);
		pEngine->device->contextI.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		pEngine->device->contextI.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
	}

	pEngine->device->contextI.Get()->Draw(6, 0);

	{
		graphics.bufferQueue.push_back(constBufferGlobalInfo);
	}
}
} 