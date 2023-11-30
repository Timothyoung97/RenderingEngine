#include "engine.h"

#include "portable-file-dialogs.h"

#define SDL_MAIN_HANDLED
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

void Engine::run() {
	// Loading Models
	std::string basePathStr = tre::Utility::getBasePathStr();													// File path
	pfd::open_file f = pfd::open_file("Choose files to read", basePathStr,
		{
			"All Files", "*" ,
			"glTF Files (.gltf)", "*.gltf",
			"obj Files (.obj)", "*.obj",
		},
		pfd::opt::force_path
		);

	if (f.result().size()) {
		ml->load(device->device.Get(), f.result()[0]);

		for (int i = 0; i < ml->_objectWithMesh.size(); i++) {
			tre::Object* pObj = ml->_objectWithMesh[i];
			scene->_pObjQ.push_back(pObj);
		}
	}

	// Stats Update
	for (int i = 0; i < scene->_pObjQ.size(); i++) {
		for (int j = 0; j < scene->_pObjQ[i]->pObjMeshes.size(); j++) {
			graphics->stats.totalMeshCount++;
			tre::Mesh* pMesh = scene->_pObjQ[i]->pObjMeshes[j];
			if ((pMesh->pMaterial->objTexture != nullptr && pMesh->pMaterial->objTexture->hasAlphaChannel)
				|| (pMesh->pMaterial->objTexture == nullptr && pMesh->pMaterial->baseColor.w < 1.0f)) {
				graphics->stats.transparentMeshCount++;
			}
			else {
				graphics->stats.opaqueMeshCount++;
			}
		}
	}

	// Delta Time between frame
	float deltaTime = 0;

	// main loop
	while (!input->shouldQuit())
	{
		MICROPROFILE_SCOPE_CSTR("Frame");

		tre::Timer timer;
		graphics->clean();
		input->updateInputEvent();
		control->update(*input, *graphics, *scene, *cam, deltaTime);
		cam->updateCamera();
		computerPtLight->compute(*graphics, *scene, *cam);
		scene->update(*graphics, *cam);
		rendererCSM->render(*graphics, *scene, *cam);
		rendererGBuffer->render(*graphics, *scene, *cam);
		rendererSSAO->render(*graphics, *scene, *cam);
		rendererEnvLighting->render(*graphics, *scene, *cam);
		rendererTransparency->render(*graphics, *scene, *cam);
		rendererLocalLighting->render(*graphics, *scene, *cam);
		rendererHDR->render(*graphics);
		rendererWireframe->render(*graphics, *cam, *scene);
		imguihelper->render();
		graphics->present();
		timer.spinWait();
		deltaTime = timer.getDeltaTime();
		profiler->recordFrame();
		profiler->storeToDisk(control->toDumpFile);
	}
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