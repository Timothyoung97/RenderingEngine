#include "computerBloom.h"

#include "engine.h"
#include "utility.h"
#include "window.h"
#include "dxdebug.h"
#include "colors.h"

extern tre::Engine* pEngine;

namespace tre {

ComputerBloom::ComputerBloom() {
	this->init();
}

void ComputerBloom::init() {
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	computeShaderDownsample.create(basePathWstr + L"shaders\\bin\\compute_shader_bloom_downsample.bin", pEngine->device->device.Get());
	computeShaderUpsample.create(basePathWstr + L"shaders\\bin\\compute_shader_bloom_upsample.bin", pEngine->device->device.Get());
}

void ComputerBloom::singleDownsample(Graphics& graphics, ID3D11Resource* pSampleTexture, ID3D11Resource* pDownsampleTexture, XMINT2 sampleViewDimension) {
	//PROFILE_GPU_SCOPED("Bloom Single Downsample");
	MICROPROFILE_SCOPE_CSTR("Bloom Single Downsample");

	ComPtr<ID3D11ShaderResourceView> sampleTextureSRV;
	{
		// create shader resource view for initial hdr texture
		D3D11_SHADER_RESOURCE_VIEW_DESC sampleTextureSRVDesc;
		sampleTextureSRVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		sampleTextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		sampleTextureSRVDesc.Texture2D= D3D11_TEX2D_SRV(0u, 1u);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
			pSampleTexture, &sampleTextureSRVDesc, sampleTextureSRV.GetAddressOf()
		));
	}

	ComPtr<ID3D11UnorderedAccessView> downsampleTextureUAV;
	{
		// create unorder access view for downsample texture
		D3D11_UNORDERED_ACCESS_VIEW_DESC downsampleTextureUAVDesc;
		downsampleTextureUAVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
		downsampleTextureUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		downsampleTextureUAVDesc.Texture2D = D3D11_TEX2D_UAV(0u);

		CHECK_DX_ERROR(pEngine->device->device.Get()->CreateUnorderedAccessView(
			pDownsampleTexture, &downsampleTextureUAVDesc, downsampleTextureUAV.GetAddressOf()
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

	// set const buffer 
	ID3D11Buffer* consBufferBloomConfig = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BloomConstBufferStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::BloomConstBufferStruct bloomConfig = {
			 XMINT2(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT),
			 XMINT2(sampleViewDimension.x * .5f, sampleViewDimension.y * .5f),
			 graphics.setting.bloomUpsampleRadius,
			 XMFLOAT3(.0f, .0f, .0f)
		};

		tre::Buffer::updateConstBufferData(contextD.Get(), consBufferBloomConfig, &bloomConfig, (UINT)sizeof(tre::BloomConstBufferStruct));
	}

	// binding
	contextD.Get()->CSSetShader(computeShaderDownsample.pShader.Get(), NULL, 0u);
	contextD.Get()->CSSetSamplers(0u, 1u, graphics._sampler.pSamplerStateMinMagMipLinearClamp.GetAddressOf());
	contextD.Get()->CSSetConstantBuffers(0u, 1u, &consBufferBloomConfig);
	contextD.Get()->CSSetShaderResources(0u, 1u, sampleTextureSRV.GetAddressOf());
	contextD.Get()->CSSetShaderResources(1u, 1u, luminAvgSRV.GetAddressOf());
	contextD.Get()->CSSetUnorderedAccessViews(0, 1u, downsampleTextureUAV.GetAddressOf(), nullptr);

	contextD.Get()->Dispatch(tre::Maths::divideAndRoundUp(sampleViewDimension.x *.5f, 4u), tre::Maths::divideAndRoundUp(sampleViewDimension.y * .5f, 4u), 1u);

	// clean up
	{
		contextD.Get()->CSSetShaderResources(0u, 1u, graphics.nullSRV);
		contextD.Get()->CSSetUnorderedAccessViews(0, 1u, graphics.nullUAV, nullptr);
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(consBufferBloomConfig);
	}
}

void ComputerBloom::downsample(Graphics& graphics) {
	//PROFILE_GPU_SCOPED("Bloom Downsample");
	MICROPROFILE_SCOPE_CSTR("Bloom Downsample");

	int writeIdx = 0;
	ID3D11Resource* pSrcResc = graphics._hdrBuffer.pHdrBufferTexture.Get();
	ID3D11Resource* pDestDesc = graphics._bloomBuffer.bloomTexture2D[writeIdx].Get();
	XMINT2 sampleViewDimension = XMINT2(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);

	for (int i = 0; i < operationCount; i++) {
		singleDownsample(graphics, pSrcResc, pDestDesc, sampleViewDimension);
		pSrcResc = graphics._bloomBuffer.bloomTexture2D[writeIdx].Get();
		writeIdx ^= 1;
		pDestDesc = graphics._bloomBuffer.bloomTexture2D[writeIdx].Get();
		sampleViewDimension = XMINT2(sampleViewDimension.x * .5f, sampleViewDimension.y * .5f);
	}
}

void ComputerBloom::singleUpsample(Graphics& graphics, ID3D11Resource* pSampleTexture, ID3D11Resource* pUpsampleTexture, XMINT2 sampleViewDimension) {
	//PROFILE_GPU_SCOPED("Bloom Single Upsample");
	MICROPROFILE_SCOPE_CSTR("Bloom Single Upsample");

	// create shader resource view for initial hdr texture
	D3D11_SHADER_RESOURCE_VIEW_DESC sampleTextureSRVDesc;
	sampleTextureSRVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	sampleTextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sampleTextureSRVDesc.Texture2D = D3D11_TEX2D_SRV(0u, 1u);

	ComPtr<ID3D11ShaderResourceView> sampleTextureSRV;
	CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
		pSampleTexture, &sampleTextureSRVDesc, sampleTextureSRV.GetAddressOf()
	));

	// create unorder access view for downsample texture
	D3D11_UNORDERED_ACCESS_VIEW_DESC upsampleTextureUAVDesc;
	upsampleTextureUAVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	upsampleTextureUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	upsampleTextureUAVDesc.Texture2D = D3D11_TEX2D_UAV(0u);

	ComPtr<ID3D11UnorderedAccessView> upsampleTextureUAV;
	CHECK_DX_ERROR(pEngine->device->device.Get()->CreateUnorderedAccessView(
		pUpsampleTexture, &upsampleTextureUAVDesc, upsampleTextureUAV.GetAddressOf()
	));

	// set const buffer 
	ID3D11Buffer* consBufferBloomConfig = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BloomConstBufferStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::BloomConstBufferStruct bloomConfig = {
			 XMINT2(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT),
			 XMINT2(sampleViewDimension.x, sampleViewDimension.y),
			 graphics.setting.bloomUpsampleRadius,
			 XMFLOAT3(.0f, .0f, .0f)
		};

		tre::Buffer::updateConstBufferData(contextD.Get(), consBufferBloomConfig, &bloomConfig, (UINT)sizeof(tre::BloomConstBufferStruct));
	}

	// binding
	contextD.Get()->CSSetShader(computeShaderUpsample.pShader.Get(), NULL, 0u);
	contextD.Get()->CSSetSamplers(0u, 1u, graphics._sampler.pSamplerStateMinMagMipLinearClamp.GetAddressOf());
	contextD.Get()->CSSetConstantBuffers(0u, 1u, &consBufferBloomConfig);
	contextD.Get()->CSSetShaderResources(0u, 1u, sampleTextureSRV.GetAddressOf());
	contextD.Get()->CSSetUnorderedAccessViews(0, 1u, upsampleTextureUAV.GetAddressOf(), nullptr);

	contextD.Get()->Dispatch(tre::Maths::divideAndRoundUp(sampleViewDimension.x, 8u), tre::Maths::divideAndRoundUp(sampleViewDimension.y, 8u), 1u);

	// clean up
	{
		contextD.Get()->CSSetShaderResources(0u, 1u, graphics.nullSRV);
		contextD.Get()->CSSetUnorderedAccessViews(0, 1u, graphics.nullUAV, nullptr);
		std::lock_guard<std::mutex> lock(graphics.bufferQueueMutex);
		graphics.bufferQueue.push_back(consBufferBloomConfig);
	}
}

void ComputerBloom::upsample(Graphics& graphics) {
	//PROFILE_GPU_SCOPED("Bloom Upsample");
	MICROPROFILE_SCOPE_CSTR("Bloom Upsample");

	int writeIdx = 1;
	ID3D11Resource* pSrcResc = graphics._bloomBuffer.bloomTexture2D[1^writeIdx].Get();
	ID3D11Resource* pDestDesc = graphics._bloomBuffer.bloomTexture2D[writeIdx].Get();

	std::vector<XMINT2> destViewDimension = { {120, 67}, {240, 135}, {480, 270}, {960, 540}, {1920, 1080} };

	for (int i = 0; i < operationCount; i++) {
		singleUpsample(graphics, pSrcResc, pDestDesc, destViewDimension[i]);
		pSrcResc = graphics._bloomBuffer.bloomTexture2D[writeIdx].Get();
		writeIdx ^= 1;
		pDestDesc = graphics._bloomBuffer.bloomTexture2D[writeIdx].Get();
	}
}

void ComputerBloom::compute(Graphics& graphics) {
	//PROFILE_GPU_SCOPED("Bloom Compute");
	MICROPROFILE_SCOPE_CSTR("Bloom Compute");
	downsample(graphics);
	upsample(graphics);

	CHECK_DX_ERROR(contextD.Get()->FinishCommandList(
		false, &commandList
	));
}

}