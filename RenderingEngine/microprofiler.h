#pragma once

#include <d3d11.h>

namespace tre {

class MicroProfiler {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	MicroProfiler(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	void recordFrame();
	void cleanup();
	void storeToDisk(bool& toStore);
};
}