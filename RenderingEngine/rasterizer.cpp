#include "rasterizer.h"
#include "dxdebug.h"

namespace tre {

Rasterizer::Rasterizer(ID3D11Device* device) {

	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 100.f;
	rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = 1.5f;
	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

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
	CHECK_DX_ERROR(device->CreateRasterizerState(
		&rasterizerDesc, pShadowRasterizerState.GetAddressOf()
	));
}

}