#pragma once

#include <d3d11.h>

#include "wrl/client.h"
#include "microprofile.h"
#include "engine.h"

using Microsoft::WRL::ComPtr;

namespace tre {

class MicroProfiler {
public:

	MicroProfiler();

	int graphicsQueue = -1;
	MicroProfileThreadLogGpu* graphicsGpuThreadLog[numOfContext];
	uint64_t graphicsMicroProfile[numOfContext];
	uint64_t graphicsTokenGpuFrameIndex[numOfContext];

	int computesQueue = -1;
	uint64_t computesTokenGpuFrameIndex[numOfComputesContext];

	void init();
	void recordFrame();
	void cleanup();
	void storeToDisk(bool& toStore);
};
}