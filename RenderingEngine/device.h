#pragma once

#include <d3d11.h>
#include "dxdebug.h"

namespace tre {
class Device {

public:
	ID3D11Device* device;
	ID3D11DeviceContext* context;

	Device();

	void InitDXDevice();
	
	~Device();
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;

};
}