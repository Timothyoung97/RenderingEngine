#pragma once

namespace tre {

class Window;
class Device;
class MicroProfiler;
class Scene;
class Camera;
class ModelLoader;
class Graphics;
class RendererCSM;
class RendererEnvironmentLighting;
class RendererGBuffer;
class RendererHDR;
class RendererLocalLighting;
class RendererSSAO;
class RendererTransparency;
class RendererWireframe;
class ComputerPointLight;
class Input;
class Control;

class Engine {
public:
	
	Window* window;
	Device* device;
	MicroProfiler* profiler;
	Scene* scene;
	Camera* cam;
	ModelLoader* ml;
	Graphics* graphics;
	RendererCSM* rendererCSM;
	RendererEnvironmentLighting* rendererEnvLighting;
	RendererGBuffer* rendererGBuffer;
	RendererHDR* rendererHDR;
	RendererLocalLighting* rendererLocalLighting;
	RendererSSAO* rendererSSAO;
	RendererTransparency* rendererTransparency;
	RendererWireframe* rendererWireframe;
	ComputerPointLight* computerPtLight;
	Input* input;
	Control* control;

	void init();
	void close();

};
}