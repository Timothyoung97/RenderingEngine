#include <spdlog/spdlog.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <string>

#include "texture.h"
#include "dxdebug.h"

namespace tre {

Texture::Texture(ID3D11Device* device, std::string filepath) {
	createTexture(device, filepath);
}

void Texture::createTexture(ID3D11Device* device, std::string filepath) {

	// load image
	int imgWidth, imgHeight, imgChannels, desiredChannels = 4;
	unsigned char* img = stbi_load(filepath.c_str(), &imgWidth, &imgHeight, &imgChannels, desiredChannels);
	if (img == NULL) {
		spdlog::error("Error loading image");
	}
	spdlog::info("Img width: {}, Img height: {}, Img channels: {}\n", imgWidth, imgHeight, imgChannels);

	// Create texture
	D3D11_TEXTURE2D_DESC texture2dDesc;
	texture2dDesc.Width = imgWidth;
	texture2dDesc.Height = imgHeight;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2dDesc.Usage = D3D11_USAGE_DYNAMIC;
	texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture2dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texture2dDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA textureData = {};
	textureData.pSysMem = img;
	textureData.SysMemPitch = imgWidth * 4;
	textureData.SysMemSlicePitch = textureData.SysMemPitch * imgHeight;

	ComPtr<ID3D11Texture2D> pTexture;

	CHECK_DX_ERROR(device->CreateTexture2D(
		&texture2dDesc, &textureData, pTexture.GetAddressOf()
	));

	STBI_FREE(img);

	// Create Shader Resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
	shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, -1);

	CHECK_DX_ERROR(device->CreateShaderResourceView(
		pTexture.Get(), &shaderResViewDesc, pShaderResView.GetAddressOf()
	));

	// Create sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	CHECK_DX_ERROR(device->CreateSamplerState(
		&samplerDesc, pSamplerState.GetAddressOf()
	));

}

}