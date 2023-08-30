#include "swapchain.h"
#include "dxdebug.h"

using Microsoft::WRL::ComPtr;

namespace tre {

Swapchain::Swapchain() {};

void Swapchain::DescSwapchain(int screenWidth, int screenHeight) {

	swapChainDesc = {};
	swapChainDesc.Width = screenWidth;
	swapChainDesc.Height = screenHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;
}

void Swapchain::InitSwapchainViaHwnd(ComPtr<IDXGIFactory2> dxgiFactory, ComPtr<ID3D11Device> device, HWND window) {
	
	CHECK_DX_ERROR(dxgiFactory->CreateSwapChainForHwnd(
		device.Get(), window, &swapChainDesc, NULL, NULL, tempSwapchain.GetAddressOf()));

	CHECK_DX_ERROR(tempSwapchain.As(&mainSwapchain));
}
}