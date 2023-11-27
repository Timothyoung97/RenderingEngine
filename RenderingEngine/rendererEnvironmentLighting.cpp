#include "rendererEnvironmentLighting.h"

#include "utility.h"

namespace tre {

RendererEnvironmentLighting::RendererEnvironmentLighting(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", _device);
	_deferredShaderLightingEnv.create(basePathWstr + L"shaders\\bin\\pixel_shader_deferred_lighting_env.bin", _device);
}

void RendererEnvironmentLighting::render(const Graphics& graphics) {
	const char* name = ToString(RENDER_MODE::DEFERRED_OPAQUE_LIGHTING_ENV_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Deferred Environment Lighting Pass");

	{
		_context->IASetInputLayout(nullptr);
		_context->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_deferredShaderLightingEnv.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(0, 1, graphics._gBuffer.pShaderResViewDeferredAlbedo.GetAddressOf()); // albedo
		_context->PSSetShaderResources(1, 1, graphics._gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		_context->PSSetShaderResources(3, 1, graphics._depthbuffer.pShadowShaderRescView.GetAddressOf()); // shadow
		_context->PSSetShaderResources(4, 1, graphics._depthbuffer.pDepthStencilShaderRescView.GetAddressOf()); //depth
		_context->PSSetShaderResources(7, 1, graphics._ssao.ssaoBlurredTexture2dSRV.GetAddressOf()); // ssao

		_context->OMSetRenderTargets(1, graphics._hdrBuffer.pRenderTargetViewHdrTexture.GetAddressOf(), nullptr);
		_context->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);
	}

	_context->Draw(6, 0);
}
} 