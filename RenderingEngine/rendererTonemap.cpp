#include "rendererTonemap.h"

#include "utility.h"
#include "window.h"
#include "engine.h"

extern tre::Engine* pEngine;

namespace tre {

RendererTonemap::RendererTonemap() {
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

	{
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(constBufferTonemap);
	}
}

void RendererTonemap::fullscreenPass(const Graphics& graphics) {
	const char* name = ToString(RENDER_MODE::TONE_MAPPING_PASS);
	MICROPROFILE_SCOPE_CSTR(name);
	//PROFILE_GPU_SCOPED("Tone Mapping Fullscreen Pass");

	// Create Vew
	ComPtr<ID3D11ShaderResourceView> sampleBloomTextureSRV;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC sampleBloomTextureSRVDesc;
		sampleBloomTextureSRVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		sampleBloomTextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		sampleBloomTextureSRVDesc.Texture2D = D3D11_TEX2D_SRV(0u, 1u);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._bloomBuffer.bloomTexture2D[1].Get(), &sampleBloomTextureSRVDesc, sampleBloomTextureSRV.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> shaderResViewHdrTexture;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
		shaderResViewDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, 1);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._hdrBuffer.pHdrBufferTexture.Get(), &shaderResViewDesc, shaderResViewHdrTexture.GetAddressOf()
		));
	}

	ComPtr<ID3D11ShaderResourceView> luminAvgSRV;
	{
		D3D11_BUFFER_SRV pLuminAvgBufferSRV;
		pLuminAvgBufferSRV.NumElements = 1;
		pLuminAvgBufferSRV.FirstElement = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC pLuminAvgSRVResc;
		pLuminAvgSRVResc.Format = DXGI_FORMAT_R16_FLOAT;
		pLuminAvgSRVResc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		pLuminAvgSRVResc.Buffer = pLuminAvgBufferSRV;

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			graphics._hdrBuffer.pLuminAvg.Get(), &pLuminAvgSRVResc, luminAvgSRV.GetAddressOf()
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
		contextD.Get()->PSSetShader(_tonemapPixelShader.pShader.Get(), NULL, 0u);
		contextD.Get()->PSSetShaderResources(0u, 1u, shaderResViewHdrTexture.GetAddressOf()); // hdr texture
		contextD.Get()->PSSetShaderResources(1u, 1u, luminAvgSRV.GetAddressOf());
		contextD.Get()->PSSetShaderResources(2u, 1u, sampleBloomTextureSRV.GetAddressOf());

		contextD.Get()->OMSetBlendState(graphics._blendstate.opaque.Get(), NULL, 0xffffffff);
		contextD.Get()->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithDepthTWriteDisabled.Get(), 0); // by default: read only depth test
		contextD.Get()->OMSetRenderTargets(1, graphics.currRenderTargetView.GetAddressOf(), nullptr);
	}

	contextD.Get()->Draw(6, 0);
}

void RendererTonemap::render(Graphics& graphics, MicroProfiler& profiler) {
	MICROPROFILE_SCOPE_CSTR("Tonemap Section");
	profiler.graphicsGpuThreadLogStatus[6] = 1;
	MICROPROFILE_CONDITIONAL(MicroProfileThreadLogGpu* pMicroProfileLog = profiler.graphicsGpuThreadLog[6]);
	MICROPROFILE_GPU_BEGIN(contextD.Get(), pMicroProfileLog);

	setConstBufferTonemap(graphics);

	{
		MICROPROFILE_SECTIONGPUI_L(pMicroProfileLog, "Tonemap Section", tre::Utility::getRandomInt(INT_MAX));
		MICROPROFILE_SCOPEGPU_TOKEN_L(pMicroProfileLog, profiler.graphicsTokenGpuFrameIndex[6]);
		MICROPROFILE_SCOPEGPUI_L(pMicroProfileLog, "Tonemap: Fullscreen Pass", tre::Utility::getRandomInt(INT_MAX));
		fullscreenPass(graphics);
	}

	CHECK_DX_ERROR(contextD->FinishCommandList(
		false, &commandList
	));

	profiler.graphicsMicroProfile[6] = MICROPROFILE_GPU_END(pMicroProfileLog);
}
}