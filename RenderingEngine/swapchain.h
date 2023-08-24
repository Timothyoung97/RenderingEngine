#pragma once

#include <dxgi1_4.h>
#include "wrl/client.h"

namespace tre {
class Swapchain {

public:
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain;

	Swapchain();

	void InitSwapchain();
};
}