#include "rendererTonemap.h"

#include "utility.h"
#include "window.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

RendererTonemap::RendererTonemap() : RendererBase() {
	this->init();
}

void RendererTonemap::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();

	_vertexShaderFullscreenQuad.create(basePathWstr + L"shaders\\bin\\vertex_shader_fullscreen.bin", pEngine->device->device.Get());
	_tonemapPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_tone_map.bin", pEngine->device->device.Get());
}

TonemapStruct RendererTonemap::createTonemapStruct(float middleGrey, float bloomStrength) {
	TonemapStruct tonemapStruct;
	tonemapStruct.middleGrey = middleGrey;
	tonemapStruct.bloomStrength = bloomStrength;
	return tonemapStruct;
}

void RendererTonemap::setConstBufferTonemap(Graphics& graphics) {
	// HDR Middle Grey
	ID3D11Buffer* constBufferTonemap = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::TonemapStruct));
	{
		// HDR const buffer update and binding
		tre::TonemapStruct tonemapStruct = createTonemapStruct(graphics.setting.middleGrey, graphics.setting.bloomStrength);
		tre::Buffer::updateConstBufferData(contextD.Get(), constBufferTonemap, &tonemapStruct, (UINT)sizeof(tre::TonemapStruct));
		contextD.Get()->PSSetConstantBuffers(0u, 1u, &constBufferTonemap);
	}
	graphics.bufferQueue.push_back(constBufferTonemap);
}

void RendererTonemap::fullscreenPass(const Graphics& graphics) {
	const char* name = ToString(RENDER_MODE::TONE_MAPPING_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Tone Mapping Fullscreen Pass");

	D3D11_SHADER_RESOURCE_VIEW_DESC sampleBloomTextureSRVDesc;
	sampleBloomTextureSRVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	sampleBloomTextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sampleBloomTextureSRVDesc.Texture2D = D3D11_TEX2D_SRV(0u, 1u);

	ComPtr<ID3D11ShaderResourceView> sampleBloomTextureSRV;
	CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
		graphics._bloomBuffer.bloomTexture2D[1].Get(), &sampleBloomTextureSRVDesc, sampleBloomTextureSRV.GetAddressOf()
	));

	// Context Confiuration
	{
		contextD.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contextD.Get()->IASetInputLayout(nullptr);
		contextD.Get()->VSSetShader(_vertexShaderFullscreenQuad.pShader.Get(), NULL, 0u);

		contextD.Get()->RSSetViewports(1, &graphics._viewport.defaultViewport);
		contextD.Get()->RSSetState(graphics._rasterizer.pRasterizerStateFCCW.Get());

		contextD.Get()->OMSetRenderTargets(0, nullptr, nullptr);
		contextD.Get()->PSSetShader(_tonemapPixelShader.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetShaderResources(0u, 1u, graphics._hdrBuffer.pShaderResViewHdrTexture.GetAddressOf()); // hdr texture
		contextD.Get()->PSSetShaderResources(1u, 1u, graphics._hdrBuffer.pLuminAvgSRV.GetAddressOf());
		contextD.Get()->PSSetShaderResources(2u, 1u, sampleBloomTextureSRV.GetAddressOf());

		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		contextD.Get()->OMSetRenderTargets(1, graphics.currRenderTargetView.GetAddressOf(), nullptr);
	}

	contextD.Get()->Draw(6, 0);
}

void RendererTonemap::render(Graphics& graphics) {
	PROFILE_GPU_SCOPED("Tonemap");
	MICROPROFILE_SCOPE_CSTR("Tonemap");
	setConstBufferTonemap(graphics);
	fullscreenPass(graphics);

	CHECK_DX_ERROR(contextD->FinishCommandList(
		false, &commandList
	));
}
}