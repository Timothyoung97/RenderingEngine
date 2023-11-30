#include "engine.h"

#include "timer.h"
#include "device.h"
#include "utility.h"
#include "window.h"
#include "microprofiler.h"
#include "scene.h"
#include "camera.h"

namespace tre {

void Engine::init() {
	srand((uint32_t)time(NULL));																	
	window = new Window{ "RenderingEngine", tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT };
	device = new Device;
	profiler = new MicroProfiler;
	scene = new Scene;
	cam = new Camera;
}

void Engine::close() {
	delete cam;
	delete scene;
	profiler->cleanup();
	delete profiler;
	delete device;
	delete window;
}

}