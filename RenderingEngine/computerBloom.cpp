#include "computerBloom.h"

#include "engine.h"
#include "utility.h"
#include "window.h"
#include "dxdebug.h"

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

	// create shader resource view for initial hdr texture
	D3D11_SHADER_RESOURCE_VIEW_DESC sampleTextureSRVDesc;
	sampleTextureSRVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	sampleTextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sampleTextureSRVDesc.Texture2D= D3D11_TEX2D_SRV(0u, 1u);

	ComPtr<ID3D11ShaderResourceView> sampleTextureSRV;
	CHECK_DX_ERROR(pEngine->device->device.Get()->CreateShaderResourceView(
		pSampleTexture, &sampleTextureSRVDesc, sampleTextureSRV.GetAddressOf()
	));

	// create unorder access view for downsample texture
	D3D11_UNORDERED_ACCESS_VIEW_DESC downsampleTextureUAVDesc;
	downsampleTextureUAVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	downsampleTextureUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	downsampleTextureUAVDesc.Texture2D = D3D11_TEX2D_UAV(0u);

	ComPtr<ID3D11UnorderedAccessView> downsampleTextureUAV;
	CHECK_DX_ERROR(pEngine->device->device.Get()->CreateUnorderedAccessView(
		pDownsampleTexture, &downsampleTextureUAVDesc, downsampleTextureUAV.GetAddressOf()
	));

	// set const buffer 
	ID3D11Buffer* consBufferBloomConfig = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BloomConstBufferStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::BloomConstBufferStruct bloomConfig = {
			 sampleViewDimension,
			 XMINT2(sampleViewDimension.x * .5f, sampleViewDimension.y * .5f),
			 2.f,
			 XMFLOAT3(.0f, .0f, .0f)
		};

		tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), consBufferBloomConfig, &bloomConfig, (UINT)sizeof(tre::BloomConstBufferStruct));
	}

	// binding
	pEngine->device->context.Get()->CSSetShader(computeShaderDownsample.pShader.Get(), NULL, 0u);
	pEngine->device->context.Get()->CSSetSamplers(0u, 1u, graphics._sampler.pSamplerStateMinMagMipLinearClamp.GetAddressOf());
	pEngine->device->context.Get()->CSSetConstantBuffers(0u, 1u, &consBufferBloomConfig);
	pEngine->device->context.Get()->CSSetShaderResources(0u, 1u, sampleTextureSRV.GetAddressOf());
	pEngine->device->context.Get()->CSSetUnorderedAccessViews(0, 1u, downsampleTextureUAV.GetAddressOf(), nullptr);

	pEngine->device->context.Get()->Dispatch(tre::Maths::divideAndRoundUp(sampleViewDimension.x *.5f, 4u), tre::Maths::divideAndRoundUp(sampleViewDimension.y * .5f, 4u), 1u);

	// clean up
	{
		graphics.bufferQueue.push_back(consBufferBloomConfig);
		pEngine->device->context.Get()->CSSetShaderResources(0u, 1u, graphics.nullSRV);
		pEngine->device->context.Get()->CSSetUnorderedAccessViews(0, 1u, graphics.nullUAV, nullptr);

	}

}

void ComputerBloom::downsample(Graphics& graphics) {
	
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

void ComputerBloom::singleUpsample(Graphics& graphics, ID3D11Resource* pSampleTexture, ID3D11Resource* pDownsampleTexture, XMINT2 sampleViewDimension) {
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
	D3D11_UNORDERED_ACCESS_VIEW_DESC downsampleTextureUAVDesc;
	downsampleTextureUAVDesc.Format = DXGI_FORMAT_R11G11B10_FLOAT;
	downsampleTextureUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	downsampleTextureUAVDesc.Texture2D = D3D11_TEX2D_UAV(0u);

	ComPtr<ID3D11UnorderedAccessView> downsampleTextureUAV;
	CHECK_DX_ERROR(pEngine->device->device.Get()->CreateUnorderedAccessView(
		pDownsampleTexture, &downsampleTextureUAVDesc, downsampleTextureUAV.GetAddressOf()
	));

	// set const buffer 
	ID3D11Buffer* consBufferBloomConfig = tre::Buffer::createConstBuffer(pEngine->device->device.Get(), (UINT)sizeof(tre::BloomConstBufferStruct));
	{
		// Create struct info and submit data to constant buffer
		tre::BloomConstBufferStruct bloomConfig = {
			 sampleViewDimension,
			 XMINT2(sampleViewDimension.x * 2.f, sampleViewDimension.y * 2.f),
			 2.f,
			 XMFLOAT3(.0f, .0f, .0f)
		};

		tre::Buffer::updateConstBufferData(pEngine->device->context.Get(), consBufferBloomConfig, &bloomConfig, (UINT)sizeof(tre::BloomConstBufferStruct));
	}

	// binding
	pEngine->device->context.Get()->CSSetShader(computeShaderDownsample.pShader.Get(), NULL, 0u);
	pEngine->device->context.Get()->CSSetSamplers(0u, 1u, graphics._sampler.pSamplerStateMinMagMipLinearClamp.GetAddressOf());
	pEngine->device->context.Get()->CSSetConstantBuffers(0u, 1u, &consBufferBloomConfig);
	pEngine->device->context.Get()->CSSetShaderResources(0u, 1u, sampleTextureSRV.GetAddressOf());
	pEngine->device->context.Get()->CSSetUnorderedAccessViews(0, 1u, downsampleTextureUAV.GetAddressOf(), nullptr);

	pEngine->device->context.Get()->Dispatch(tre::Maths::divideAndRoundUp(sampleViewDimension.x * .5f, 4u), tre::Maths::divideAndRoundUp(sampleViewDimension.y * .5f, 4u), 1u);

	// clean up
	{
		graphics.bufferQueue.push_back(consBufferBloomConfig);
		pEngine->device->context.Get()->CSSetShaderResources(0u, 1u, graphics.nullSRV);
		pEngine->device->context.Get()->CSSetUnorderedAccessViews(0, 1u, graphics.nullUAV, nullptr);

	}
}

void ComputerBloom::upsample(Graphics& graphics) {

}

void ComputerBloom::compute(Graphics& graphics) {

	// downsample
	downsample(graphics);
	// upsample
}

}