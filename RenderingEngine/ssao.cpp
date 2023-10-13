#include <DirectXMath.h>

#include <vector>

#include "ssao.h"
#include "utility.h"
#include "dxdebug.h"
#include "maths.h"

using namespace DirectX;

namespace tre {

void SSAO::create(ID3D11Device* device, ID3D11DeviceContext* context) {
	_device = device;
	_context = context;

	// create ssaoKernal
	std::vector<XMFLOAT3> ssaoKernalSamples;
	for (int i = 0; i < 64; i++) {
		XMVECTOR newSample{
			tre::Utility::getRandomFloatRange(-1.f, 1.f),
			tre::Utility::getRandomFloatRange(-1.f, 1.f),
			tre::Utility::getRandomFloatRange(0.f, 1.f)
		};

		newSample = XMVector3Normalize(newSample);

		float scale = (float)i / 64.0f;
		scale = tre::Maths::lerp(.1f, .1f, scale * scale);

		XMFLOAT3 sampleF;
		XMStoreFloat3(&sampleF, newSample * scale);

		ssaoKernalSamples.push_back(sampleF);
	}

	D3D11_BUFFER_DESC ssaoKernalBufferDesc;
	ssaoKernalBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ssaoKernalBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ssaoKernalBufferDesc.CPUAccessFlags = 0u;
	ssaoKernalBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	ssaoKernalBufferDesc.ByteWidth = static_cast<UINT>(sizeof(XMFLOAT3) * ssaoKernalSamples.size());
	ssaoKernalBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(XMFLOAT3));

	D3D11_SUBRESOURCE_DATA ssaoKernalData = {};
	ssaoKernalData.pSysMem = ssaoKernalSamples.data();

	CHECK_DX_ERROR(_device->CreateBuffer(
		&ssaoKernalBufferDesc, &ssaoKernalData, ssaoKernelBuffer.GetAddressOf()
	));

	D3D11_BUFFER_SRV bufferSRV;
	bufferSRV.NumElements = ssaoKernalSamples.size();
	bufferSRV.FirstElement = 0u;
	bufferSRV.ElementWidth = static_cast<UINT>(sizeof(XMFLOAT3));

	D3D11_SHADER_RESOURCE_VIEW_DESC ssaoKernalSRVDesc;
	ssaoKernalSRVDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	ssaoKernalSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	ssaoKernalSRVDesc.Buffer = bufferSRV;

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		ssaoKernelBuffer.Get(), &ssaoKernalSRVDesc, ssaoKernelBufferSRV.GetAddressOf()
	));

	// create ssaoNoise
	std::vector<XMFLOAT3> ssaoNoise;
	for (int i = 0; i < 16; i++) {
		XMFLOAT3 newNoise = XMFLOAT3(
			tre::Utility::getRandomFloatRange(-1.f, 1.f),
			tre::Utility::getRandomFloatRange(-1.f, 1.f),
			.0f
		);
		ssaoNoise.push_back(newNoise);
	}

	D3D11_TEXTURE2D_DESC ssaoNoiseTextureDesc;
	ssaoNoiseTextureDesc.Width = 4;
	ssaoNoiseTextureDesc.Height = 4;
	ssaoNoiseTextureDesc.MipLevels = 1;
	ssaoNoiseTextureDesc.ArraySize = 1;
	ssaoNoiseTextureDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	ssaoNoiseTextureDesc.SampleDesc.Count = 1;
	ssaoNoiseTextureDesc.SampleDesc.Quality = 0;
	ssaoNoiseTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	ssaoNoiseTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ssaoNoiseTextureDesc.CPUAccessFlags = 0;
	ssaoNoiseTextureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA ssaoNoiseTextureData = {};
	ssaoNoiseTextureData.pSysMem = ssaoNoise.data();
	ssaoNoiseTextureData.SysMemPitch = static_cast<UINT>(4 * sizeof(XMFLOAT3));
	ssaoNoiseTextureData.SysMemSlicePitch = 0u;

	CHECK_DX_ERROR(_device->CreateTexture2D(
		&ssaoNoiseTextureDesc, &ssaoNoiseTextureData, ssaoNoiseTexture2D.GetAddressOf()
	));

	D3D11_SHADER_RESOURCE_VIEW_DESC ssaoNoiseSRVDesc;
	ssaoNoiseSRVDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	ssaoNoiseSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ssaoNoiseSRVDesc.Texture2D = D3D11_TEX2D_SRV(0, -1);

	CHECK_DX_ERROR(_device->CreateShaderResourceView(
		ssaoNoiseTexture2D.Get(), &ssaoNoiseSRVDesc, ssaoNoiseTexture2DSRV.GetAddressOf()
	));
}
}