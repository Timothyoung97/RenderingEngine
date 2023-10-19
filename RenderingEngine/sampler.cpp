#include "sampler.h"

#include "dxdebug.h"

namespace tre {

void Sampler::create(ID3D11Device* device) {
	// Create sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	CHECK_DX_ERROR(device->CreateSamplerState(
		&samplerDesc, pSamplerStateLinear.GetAddressOf()
	));

	// Create sampler state for shadow
	D3D11_SAMPLER_DESC shadowSamplerDesc;
	ZeroMemory(&shadowSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	shadowSamplerDesc.MinLOD = 0.f;
	shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	shadowSamplerDesc.MipLODBias = 0.f;
	shadowSamplerDesc.MaxAnisotropy = 0;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

	CHECK_DX_ERROR(device->CreateSamplerState(&shadowSamplerDesc, pSamplerStateMipPtWhiteBorder.GetAddressOf()));

	// Create sampler state for ssao
	D3D11_SAMPLER_DESC ssaoSamplerDesc;
	ZeroMemory(&ssaoSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	ssaoSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	ssaoSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	ssaoSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	ssaoSamplerDesc.BorderColor[0] = 1.0f;
	ssaoSamplerDesc.BorderColor[1] = 1.0f;
	ssaoSamplerDesc.BorderColor[2] = 1.0f;
	ssaoSamplerDesc.BorderColor[3] = 1.0f;
	ssaoSamplerDesc.MinLOD = 0.f;
	ssaoSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ssaoSamplerDesc.MipLODBias = 0.f;
	ssaoSamplerDesc.MaxAnisotropy = 0;
	ssaoSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	ssaoSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

	CHECK_DX_ERROR(device->CreateSamplerState(&ssaoSamplerDesc, pSamplerStateMipPtWrap.GetAddressOf()));

	ssaoSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ssaoSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ssaoSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ssaoSamplerDesc.BorderColor[0] = 1.0f;
	ssaoSamplerDesc.BorderColor[1] = 1.0f;
	ssaoSamplerDesc.BorderColor[2] = 1.0f;
	ssaoSamplerDesc.BorderColor[3] = 1.0f;

	CHECK_DX_ERROR(device->CreateSamplerState(&ssaoSamplerDesc, pSamplerStateMipPtClamp.GetAddressOf()));
}
}