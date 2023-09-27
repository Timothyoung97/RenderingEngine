#pragma once

#include <d3d11.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {
class Swapchain {
	ComPtr<IDXGISwapChain1> tempSwapchain;

public:
	ComPtr<IDXGISwapChain3> mainSwapchain;

	void create(
		ComPtr<IDXGIFactory2> dxgiFactory,
		ComPtr<ID3D11Device> device,
		HWND window
	);
};
}