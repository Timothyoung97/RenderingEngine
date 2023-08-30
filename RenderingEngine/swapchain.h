#pragma once

#include <d3d11.h>
#include <dxgi1_4.h>
#include "wrl/client.h"

namespace tre {
class Swapchain {
	Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapchain;

public:
	Microsoft::WRL::ComPtr<IDXGISwapChain3> mainSwapchain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;

	Swapchain();

	void DescSwapchain(int screenWidth, int screenHeight);

	void InitSwapchainViaHwnd(
		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		HWND window
	);
};
}