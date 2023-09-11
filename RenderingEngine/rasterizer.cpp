#include "rasterizer.h"
#include "dxdebug.h"

namespace tre {

Rasterizer::Rasterizer(ID3D11Device* device) {
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
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


}

}