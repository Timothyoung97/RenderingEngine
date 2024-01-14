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
	MICROPROFILE_CONDITIONAL(queueGraphics = MICROPROFILE_GPU_INIT_QUEUE("GPU-Graphics-Queue"));

	MicroProfileOnThreadCreate("Main");
	MicroProfileSetEnableAllGroups(true);
	MicroProfileSetForceMetaCounters(true);

	for (uint32_t i = 0; i < _countof(tokenGpuFrameIndex); i++) {
		char frame[255];
		snprintf(frame, sizeof(frame) - 1, "Graphics-Read-%d", i);
		tokenGpuFrameIndex[i] = MicroProfileGetToken("GPU", frame, (uint32_t)-1, MicroProfileTokenTypeGpu, 0);
	}

	MicroProfileGpuInitD3D11(pEngine->device->device.Get(), pEngine->device->contextI.Get());
	MICROPROFILE_GPU_SET_CONTEXT(pEngine->device->contextI.Get(), MicroProfileGetGlobalGpuThreadLog());
	//MicroProfileStartContextSwitchTrace();

	for (int i = 0; i < numOfContext; i++) {
		gpuThreadLog[i] = MicroProfileThreadLogGpuAlloc();
	}
}

void MicroProfiler::recordFrame() {
	MicroProfileFlip(pEngine->device->contextI.Get());
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