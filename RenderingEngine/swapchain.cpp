#include "swapchain.h"

using Microsoft::WRL::ComPtr;

namespace tre {

Swapchain::Swapchain() {};

void Swapchain::InitSwapchain(int screenWidth, int screenHeight, ) {

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = SCREEN_WIDTH;
	swapChainDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	CHECK_DX_ERROR(factory.dxgiFactory2->CreateSwapChainForHwnd(
		deviceAndContext.device.Get(), window.getWindowHandle(), &swapChainDesc, NULL, NULL, &swapChain
	));

	swapChain3 = (IDXGISwapChain3*)swapChain;
}


}