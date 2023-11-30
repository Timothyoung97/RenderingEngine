#pragma once

namespace tre {

class Window;
class Device;
class MicroProfiler;
class Scene;
class Camera;

class Engine {
public:
	
	Window* window;
	Device* device;
	MicroProfiler* profiler;
	Scene* scene;
	Camera* cam;

	void init();
	void close();

};
}