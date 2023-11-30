#include "engine.h"

#include "camera.h"
#include "colors.h"
#include "computerPointLight.h"
#include "control.h"
#include "dxdebug.h"
#include "device.h"
#include "graphics.h"
#include "imguihelper.h"
#include "input.h"
#include "mesh.h"
#include "microprofiler.h"
#include "modelloader.h"
#include "object.h"
#include "rendererCSM.h"
#include "rendererEnvironmentLighting.h"
#include "rendererGBuffer.h"
#include "rendererHDR.h"
#include "rendererLocalLighting.h"
#include "rendererSSAO.h"
#include "rendererTransparency.h"
#include "rendererWireframe.h"
#include "scene.h"
#include "timer.h"
#include "utility.h"
#include "window.h"

namespace tre {

void Engine::init() {
	srand((uint32_t)time(NULL));																	
	window = new Window{ "RenderingEngine", tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT };
	device = new Device;
	profiler = new MicroProfiler;
	scene = new Scene;
	cam = new Camera;
	ml = new ModelLoader;
	graphics = new Graphics;
	rendererCSM =  new RendererCSM;
	rendererEnvLighting =  new RendererEnvironmentLighting;
	rendererGBuffer =  new RendererGBuffer;
	rendererHDR =  new RendererHDR;
	rendererLocalLighting =  new RendererLocalLighting;
	rendererSSAO =  new RendererSSAO;
	rendererTransparency =  new RendererTransparency;
	rendererWireframe =  new RendererWireframe;
	computerPtLight =  new ComputerPointLight;
	input =  new Input;
	control =  new Control;
	imguihelper = new ImguiHelper;
}

void Engine::close() {
	imguihelper->cleanup();
	delete imguihelper;

	delete input;
	delete control;
	
	delete rendererCSM;
	delete rendererEnvLighting;
	delete rendererGBuffer;
	delete rendererHDR;
	delete rendererLocalLighting;
	delete rendererSSAO;
	delete rendererTransparency;
	delete rendererWireframe;
	delete computerPtLight;

	delete graphics;
	
	delete ml;
	delete cam;
	delete scene;
	profiler->cleanup();
	delete profiler;
	delete device;
	delete window;
}

}