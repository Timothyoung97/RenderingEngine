#include "sampler.h"

#include "dxdebug.h"

namespace tre {

void Sampler::create(ID3D11Device* device) {
	// Create sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	CHECK_DX_ERROR(device->CreateSamplerState(
		&samplerDesc, pSamplerStateMinMagMipLinearWrap.GetAddressOf()
	));

	// Create sampler state used for shadow
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

	CHECK_DX_ERROR(device->CreateSamplerState(
		&samplerDesc, pSamplerStateMinMagMipLinearGreaterEqualBorder.GetAddressOf()
	));

	// Create sampler state used for ssao
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

	CHECK_DX_ERROR(device->CreateSamplerState(
		&samplerDesc, pSamplerStateMinMagMipPtWrap.GetAddressOf()
	));

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	CHECK_DX_ERROR(device->CreateSamplerState(
		&samplerDesc, pSamplerStateMinMagMipPtClamp.GetAddressOf()
	));

	// Create sampler state used for bloom downscaling
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	CHECK_DX_ERROR(device->CreateSamplerState(
		&samplerDesc, pSamplerStateMinMagMipLinearClamp.GetAddressOf()
	));


}
}