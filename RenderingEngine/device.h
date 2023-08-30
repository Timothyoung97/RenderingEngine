#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace tre {
class Device {

public:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

	Device();

	void InitDXDevice();
	
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;

};
}