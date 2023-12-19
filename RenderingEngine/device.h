#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace tre {
class Device {

public:
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> contextI;

	ID3D11CommandList* commandListQueue[11] = { nullptr };

	Device();

	void InitDXDevice();
	void executeCommandList();

	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;

};
}