#include "blendstate.h"

namespace tre {

void BlendState::create(ID3D11Device* device) {

	D3D11_BLEND_DESC blendDesc{};
	D3D11_RENDER_TARGET_BLEND_DESC rtbd{};

	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0] = rtbd;
	device->CreateBlendState(&blendDesc, opaque.GetAddressOf());

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0] = rtbd;
	device->CreateBlendState(&blendDesc, transparency.GetAddressOf());

	rtbd.SrcBlend = D3D11_BLEND_ONE;
	rtbd.DestBlend = D3D11_BLEND_ONE;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.DestBlendAlpha = D3D11_BLEND_ONE;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0] = rtbd;
	device->CreateBlendState(&blendDesc, lighting.GetAddressOf());
};

}