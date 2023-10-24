#include "swapchain.h"
#include "dxdebug.h"
#include "window.h"

using Microsoft::WRL::ComPtr;

namespace tre {

void Swapchain::create(ComPtr<IDXGIFactory6> dxgiFactory, ComPtr<ID3D11Device> device, HWND window) {

	BOOL tearingSupported{};
	CHECK_DX_ERROR(dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearingSupported, sizeof(tearingSupported)));
	m_bTearingSupported = tearingSupported;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = tre::SCREEN_WIDTH;
	swapChainDesc.Height = tre::SCREEN_HEIGHT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = m_bTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	CHECK_DX_ERROR(dxgiFactory->CreateSwapChainForHwnd(
		device.Get(), window, &swapChainDesc, NULL, NULL, tempSwapchain.GetAddressOf()));

	CHECK_DX_ERROR(tempSwapchain.As(&mainSwapchain));
}
}