#include "engine.h"

#include "window.h"
#include "device.h"
#include "microprofiler.h"

namespace tre {

void Engine::init() {
	window = new Window{ "RenderingEngine", tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT };
	device = new Device;
	profiler = new MicroProfiler;
}

void Engine::close() {

	profiler->cleanup();
	delete profiler;

	delete device;
	delete window;
}

}