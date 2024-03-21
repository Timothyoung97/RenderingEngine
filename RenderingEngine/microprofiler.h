#pragma once

#include <d3d11.h>
#include <memory>

#include "wrl/client.h"
#include "microprofile.h"
#include "engine.h"

using Microsoft::WRL::ComPtr;

namespace tre {

class MicroProfiler {
public:

	MicroProfiler();

	int graphicsQueue = -1;
	int graphicsGpuThreadLogStatus[numOfGraphicsContext];
	MicroProfileThreadLogGpu* graphicsGpuThreadLog[numOfGraphicsContext];
	uint64_t graphicsMicroProfile[numOfGraphicsContext];
	uint64_t graphicsTokenGpuFrameIndex[numOfGraphicsContext];

	int computesQueue = -1;
	uint64_t computesTokenGpuFrameIndex[numOfComputesContext];

	void init();
	void recordFrame();
	void cleanup();
	void storeToDisk(bool& toStore);
};
}