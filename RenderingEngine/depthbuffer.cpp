#include "depthbuffer.h"
#include "dxdebug.h"

namespace tre {

DepthBuffer::DepthBuffer(ID3D11Device* device, int screenW, int screenH) {
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = screenW;
	depthStencilDesc.Height = screenH;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	CHECK_DX_ERROR(device->CreateTexture2D(
		&depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf()
	));

	CHECK_DX_ERROR(device->CreateDepthStencilView(
		depthStencilBuffer.Get(), nullptr, depthStencilView.GetAddressOf()
	));
}
}