#include "microprofiler.h"

#include "microprofile.h"

#include <stdio.h>

namespace tre {

MicroProfiler::MicroProfiler(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	MicroProfileOnThreadCreate("Main");
	MicroProfileSetEnableAllGroups(true);
	MicroProfileSetForceMetaCounters(true);

	MicroProfileGpuInitD3D11(_device, _context);
	MICROPROFILE_GPU_SET_CONTEXT(_context, MicroProfileGetGlobalGpuThreadLog());
	MicroProfileStartContextSwitchTrace();
}

void MicroProfiler::recordFrame() {
	MicroProfileFlip(_context);
}

void MicroProfiler::cleanup() {
	MicroProfileGpuShutdown();
	MicroProfileShutdown();
}

void MicroProfiler::storeToDisk(bool& toStore) {
	if (toStore) {
		printf("Profiling\n");
		MicroProfileDumpFileImmediately("profile.html", nullptr, _context);
		toStore = false;
	}
}
}