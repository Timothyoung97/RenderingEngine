#pragma once

#include <d3d11.h>

#include "microprofile.h"
#include "engine.h"

namespace tre {

class MicroProfiler {
public:

	MicroProfiler();

	int queueGraphics = -1;
	uint64_t tokenGpuFrameIndex[2];
	UINT nSrc = 0;
	UINT nDest = 1;

	uint64_t microProfile[numOfContext];

	MicroProfileThreadLogGpu* gpuThreadLog[numOfContext];

	void init();
	void recordFrame();
	void cleanup();
	void storeToDisk(bool& toStore);
};
}