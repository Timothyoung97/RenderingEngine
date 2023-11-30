#pragma once

namespace tre {

class Window;
class Device;
class MicroProfiler;

class Engine {
public:
	
	Window* window;
	Device* device;
	MicroProfiler* profiler;

	void init();
	void close();

};
}