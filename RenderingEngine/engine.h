#pragma once
#include <vector>

namespace tre {

const static int numOfContext = 11;
const static int numOfComputesContext = 3;
const static int numOfGraphicsContext = 8;

class Window;
class Device;
class MicroProfiler;
class Scene;
class Camera;
class ModelLoader;
class Graphics;
class RendererBase;
class RendererCSM;
class RendererEnvironmentLighting;
class RendererGBuffer;
class RendererLocalLighting;
class RendererSSAO;
class RendererTransparency;
class RendererTonemap;
class RendererWireframe;
class ComputerPointLight;
class ComputerBloom;
class ComputerHDR;
class Input;
class Control;
class ImguiHelper;

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
	RendererLocalLighting* rendererLocalLighting;
	RendererSSAO* rendererSSAO;
	RendererTransparency* rendererTransparency;
	RendererWireframe* rendererWireframe;
	RendererTonemap* rendererTonemap;
	ComputerHDR* computerHDR;
	ComputerPointLight* computerPtLight;
	ComputerBloom* computerBloom;
	Input* input;
	Control* control;
	ImguiHelper* imguihelper;

	std::vector<RendererBase*> renderers;

	void init();
	void run();
	void deleteCommandList();
	void executeCommandList();
	void close();

};
}
