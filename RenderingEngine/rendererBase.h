#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "device.h"
#include "graphics.h"

using Microsoft::WRL::ComPtr;

namespace tre {
class RendererBase {
public:
	ComPtr<ID3D11DeviceContext> contextD;

	RendererBase();

	virtual void init();
};
}