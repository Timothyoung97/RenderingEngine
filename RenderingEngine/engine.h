#pragma once

namespace tre {

class Window;
class Device;
class MicroProfiler;
class Scene;

class Engine {
public:
	
	Window* window;
	Device* device;
	MicroProfiler* profiler;
	Scene* scene;

	void init();
	void close();

};
}