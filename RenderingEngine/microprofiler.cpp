#include "microprofiler.h"

#include "microprofile.h"

#include <stdio.h>

#include "engine.h"
#include "device.h"

extern tre::Engine* pEngine;

namespace tre {

MicroProfiler::MicroProfiler() {
	this->init();
}

void MicroProfiler::init() {

	MICROPROFILE_CONDITIONAL(graphicsQueue = MICROPROFILE_GPU_INIT_QUEUE("GPU-Graphics-Queue"));
	MICROPROFILE_CONDITIONAL(computesQueue = MICROPROFILE_GPU_INIT_QUEUE("GPU-Computes-Queue"));

	MicroProfileOnThreadCreate("Main");
	MicroProfileSetEnableAllGroups(true);
	MicroProfileSetForceMetaCounters(true);

	for (uint32_t i = 0; i < _countof(graphicsTokenGpuFrameIndex); i++) {
		char frame[255];
		snprintf(frame, sizeof(frame) - 1, "Graphics-Thread-%d", i);
		graphicsTokenGpuFrameIndex[i] = MicroProfileGetToken("GPU", frame, (uint32_t)-1, MicroProfileTokenTypeGpu, 0);
	}

	for (uint32_t i = 0; i < _countof(computesTokenGpuFrameIndex); i++) {
		char frame[255];
		snprintf(frame, sizeof(frame) - 1, "Computes-Thread-%d", i);
		computesTokenGpuFrameIndex[i] = MicroProfileGetToken("GPU", frame, (uint32_t)-1, MicroProfileTokenTypeGpu, 0);
	}

	MicroProfileGpuInitD3D11(pEngine->device->device.Get(), pEngine->device->contextI.Get());
	MICROPROFILE_GPU_SET_CONTEXT(pEngine->device->contextI.Get(), MicroProfileGetGlobalGpuThreadLog());

	for (int i = 0; i < numOfGraphicsContext; i++) {
		graphicsGpuThreadLog[i] = MicroProfileThreadLogGpuAlloc();
	}
}

void MicroProfiler::recordFrame() {
	MicroProfileFlip(pEngine->device->contextI.Get());
	MicroProfileGpuFlip(pEngine->device->contextI.Get());
}

void MicroProfiler::cleanup() {
	MicroProfileGpuShutdown();
	MicroProfileShutdown();
}

void MicroProfiler::storeToDisk(bool& toStore) {
	if (toStore) {
		printf("Profiling\n");
		MicroProfileDumpFileImmediately("profile.html", nullptr, pEngine->device->contextI.Get());
		toStore = false;
	}
}
}