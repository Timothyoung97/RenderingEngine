#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace tre {
class Device {

public:
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> contextI;

	Device();

	void InitDXDevice();
	
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;

};
}