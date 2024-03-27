#pragma once

#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "dxdebug.h"
#include "window.h"

using Microsoft::WRL::ComPtr;

namespace tre {
class Swapchain {
	ComPtr<IDXGISwapChain1> tempSwapchain;

public:
	ComPtr<IDXGISwapChain3> mainSwapchain;

	void create(
		ComPtr<IDXGIFactory6> dxgiFactory,
		ComPtr<ID3D11Device> device,
		HWND window
	);

	bool m_bTearingSupported = false;
};
}