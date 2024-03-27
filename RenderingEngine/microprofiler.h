#pragma once

#include <d3d11.h>
#include <stdio.h>
#include <wrl/client.h>

#include <memory>

#include "microprofile.h"
#include "engine.h"
#include "device.h"

extern std::shared_ptr<tre::Engine> pEngine;

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