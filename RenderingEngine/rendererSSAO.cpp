#include "rendererSSAO.h"
#include "utility.h"

namespace tre {

RendererSSAO::RendererSSAO(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", _device);
	_ssaoPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_ssao_rendering.bin", _device);
	_textureBlurPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_texture_blur.bin", _device);
}

SSAOKernalStruct RendererSSAO::createSSAOKernalStruct(const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius) {
	SSAOKernalStruct ssaoKernalStruct;
	std::copy(kernalSamples.begin(), kernalSamples.end(), ssaoKernalStruct.kernalSamples);
	ssaoKernalStruct.sampleRadius = sampleRadius;

	return ssaoKernalStruct;
}

void RendererSSAO::setConstBufferSSAOSetting(Graphics& graphics) {
	if (!graphics.setting.ssaoSwitch) return;

	ID3D11Buffer* constBufferSSAOKernal = tre::Buffer::createConstBuffer(_device, (UINT)sizeof(tre::SSAOKernalStruct));;
	tre::SSAOKernalStruct ssaoKernalStruct = createSSAOKernalStruct(graphics._ssao.ssaoKernalSamples, graphics.setting.ssaoSampleRadius);
	tre::Buffer::updateConstBufferData(_context, constBufferSSAOKernal, &ssaoKernalStruct, (UINT)sizeof(tre::SSAOKernalStruct));

	_context->PSSetConstantBuffers(3u, 1u, &constBufferSSAOKernal);

	graphics.bufferQueue.push_back(constBufferSSAOKernal);
}

void RendererSSAO::fullscreenPass(const Graphics& graphics) {
	if (!graphics.setting.ssaoSwitch) return;

	const char* name = ToString(RENDER_MODE::SSAO_FULLSCREEN_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Fullscreen Pass");

	{
		_context->IASetInputLayout(nullptr);
		_context->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_ssaoPixelShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(1, 1, graphics._gBuffer.pShaderResViewDeferredNormal.GetAddressOf()); // normal
		_context->PSSetShaderResources(4, 1, graphics._depthbuffer.pDepthStencilShaderRescView.GetAddressOf()); //depth

		_context->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		_context->OMSetRenderTargets(1, graphics._ssao.ssaoResultTexture2dRTV.GetAddressOf(), nullptr);
	}

	_context->Draw(6, 0);
}

void RendererSSAO::fullscreenBlurPass(const Graphics& graphics) {
	if (!graphics.setting.ssaoSwitch) return;

	const char* name = ToString(RENDER_MODE::SSAO_BLURRING_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Fullscreen Pass");

	{
		_context->IASetInputLayout(nullptr);
		_context->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		_context->OMSetRenderTargets(0, nullptr, nullptr);
		_context->PSSetShader(_textureBlurPixelShader.pShader.Get(), NULL, 0u);
		_context->PSSetShaderResources(6, 1, graphics._ssao.ssaoResultTexture2dSRV.GetAddressOf()); // normal

		_context->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		_context->OMSetRenderTargets(1, graphics._ssao.ssaoBlurredTexture2dRTV.GetAddressOf(), nullptr);
	}

	_context->Draw(6, 0);
}

}