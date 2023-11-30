#pragma once

#include <d3d11.h>

namespace tre {

class MicroProfiler {
public:

	MicroProfiler();

	void init();
	void recordFrame();
	void cleanup();
	void storeToDisk(bool& toStore);
};
}