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
	MicroProfileOnThreadCreate("Main");
	MicroProfileSetEnableAllGroups(true);
	MicroProfileSetForceMetaCounters(true);


	MicroProfileGpuInitD3D11(pEngine->device->device.Get(), pEngine->device->contextI.Get());
	MICROPROFILE_GPU_SET_CONTEXT(pEngine->device->contextI.Get(), MicroProfileGetGlobalGpuThreadLog());
	MicroProfileStartContextSwitchTrace();
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