#pragma once

#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <wrl/client.h>

#include "dxdebug.h"

using Microsoft::WRL::ComPtr;

namespace tre {
class Factory {

// DXGI_DEBUG_ALL
const GUID dxgi_debug_all = { 0xe48ae283, 0xda80, 0x490b, { 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 } };

public:
	ComPtr<IDXGIFactory2> dxgiFactory2;

	void create();

};
}