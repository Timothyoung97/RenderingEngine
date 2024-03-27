#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "device.h"
#include "graphics.h"
#include "engine.h"
#include "dxdebug.h"

extern std::shared_ptr<tre::Engine> pEngine;

using Microsoft::WRL::ComPtr;

namespace tre {
class RendererBase {
public:
	ComPtr<ID3D11DeviceContext> contextD;
	ID3D11CommandList* commandList = nullptr;

	RendererBase();

	virtual void init();
};
}