#include "rasterizer.h"

namespace tre {

void Rasterizer::create(ID3D11Device* device) {

	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;
	CHECK_DX_ERROR(device->CreateRasterizerState(
		&rasterizerDesc, pRasterizerStateFCW.GetAddressOf()
	));

	rasterizerDesc.FrontCounterClockwise = true;
	CHECK_DX_ERROR(device->CreateRasterizerState(
		&rasterizerDesc, pRasterizerStateFCCW.GetAddressOf()
	));

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	CHECK_DX_ERROR(device->CreateRasterizerState(
		&rasterizerDesc, pRasterizerStateNoCull.GetAddressOf()
	));

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	CHECK_DX_ERROR(device->CreateRasterizerState(
		&rasterizerDesc, pRasterizerStateWireFrame.GetAddressOf()
	));

	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = true;
	rasterizerDesc.DepthBias = 5.f;
	rasterizerDesc.DepthBiasClamp = -1.0f;
	rasterizerDesc.SlopeScaledDepthBias = 1.2f;
	CHECK_DX_ERROR(device->CreateRasterizerState(
		&rasterizerDesc, pShadowRasterizerState.GetAddressOf()
	));
}

}