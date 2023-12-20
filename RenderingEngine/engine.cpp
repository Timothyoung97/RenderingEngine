#include "engine.h"

#include "portable-file-dialogs.h"

#define SDL_MAIN_HANDLED
#include "camera.h"
#include "colors.h"
#include "computerBloom.h"
#include "computerHDR.h"
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
#include "rendererTonemap.h"
#include "rendererLocalLighting.h"
#include "rendererSSAO.h"
#include "rendererTransparency.h"
#include "rendererWireframe.h"
#include "scene.h"
#include "timer.h"
#include "utility.h"
#include "window.h"
#include "taskflow/taskflow.hpp"

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
	rendererLocalLighting =  new RendererLocalLighting;
	computerHDR =  new ComputerHDR;
	rendererTonemap = new RendererTonemap;
	rendererSSAO =  new RendererSSAO;
	rendererTransparency =  new RendererTransparency;
	rendererWireframe =  new RendererWireframe;
	computerPtLight =  new ComputerPointLight;
	computerBloom = new ComputerBloom;
	input =  new Input;
	control =  new Control;
	imguihelper = new ImguiHelper;

#ifdef _DEBUG
	D3D11_FEATURE_DATA_THREADING featureCheckStruct;
	CHECK_DX_ERROR(device->device.Get()->CheckFeatureSupport(
		D3D11_FEATURE_THREADING, &featureCheckStruct, (UINT)sizeof(D3D11_FEATURE_DATA_THREADING)
	));

	printf("Command List Support: %s\n", featureCheckStruct.DriverCommandLists ? "True" : "False");
	printf("Concurrent Create Supported: %s\n", featureCheckStruct.DriverConcurrentCreates ? "True" : "False");
#endif
}

void Engine::executeCommandList() {
	device->contextI->ExecuteCommandList(rendererCSM->commandList, false);
	rendererCSM->commandList->Release();

	if (scene->_culledOpaqueObjQ[scene->camViewIdx].size()) {
		device->contextI->ExecuteCommandList(rendererGBuffer->commandList, false);
		rendererGBuffer->commandList->Release();
	}

	if (graphics->setting.ssaoSwitch) {
		device->contextI->ExecuteCommandList(rendererSSAO->commandList, false);
		rendererSSAO->commandList->Release();
	}

	device->contextI->ExecuteCommandList(rendererEnvLighting->commandList, false);
	rendererEnvLighting->commandList->Release();

	if (scene->_culledTransparentObjQ.size()) {
		device->contextI->ExecuteCommandList(rendererTransparency->commandList, false);
		rendererTransparency->commandList->Release();
	}

	if (scene->lightResc.numOfLights) {
		device->contextI->ExecuteCommandList(rendererLocalLighting->commandList, false);
		rendererLocalLighting->commandList->Release();

		device->contextI->ExecuteCommandList(computerPtLight->commandList, false);
		computerPtLight->commandList->Release();
	}

	device->contextI->ExecuteCommandList(computerHDR->commandList, false);
	computerHDR->commandList->Release();

	device->contextI->ExecuteCommandList(computerBloom->commandList, false);
	computerBloom->commandList->Release();

	device->contextI->ExecuteCommandList(rendererTonemap->commandList, false);
	rendererTonemap->commandList->Release();

	if (graphics->setting.showBoundingVolume) {
		device->contextI->ExecuteCommandList(rendererWireframe->commandList, false);
		rendererWireframe->commandList->Release();
	}
}

void Engine::run() {
	// Loading Models
	std::string basePathStr = tre::Utility::getBasePathStr();													
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
			if (pMesh->pMaterial->isTransparent()) {
				graphics->stats.transparentMeshCount++;
			} else {
				graphics->stats.opaqueMeshCount++;
			}
		}
	}

	// Delta Time between frame
	float deltaTime = 0;

	tf::Executor executor;

	// main loop
	while (!input->shouldQuit())
	{
		MICROPROFILE_SCOPE_CSTR("Frame");

		tre::Timer timer;
		tf::Taskflow taskflow;

		graphics->clean();
		input->updateInputEvent();
		control->update(*input, *graphics, *scene, *cam, deltaTime);
		cam->updateCamera();
		scene->update(*graphics, *cam);
		
		taskflow.emplace(
			[this]() { rendererSSAO->render(*graphics, *scene, *cam); },
			[this]() { rendererEnvLighting->render(*graphics, *scene, *cam); },
			[this]() { rendererTransparency->render(*graphics, *scene, *cam); },
			[this]() { rendererLocalLighting->render(*graphics, *scene, *cam); },
			[this]() { computerPtLight->compute(*graphics, *scene, *cam); },
			[this]() { rendererTonemap->render(*graphics); },
			[this]() { computerHDR->compute(*graphics); },
			[this]() { computerBloom->compute(*graphics); }
		);

		{
			rendererCSM->render(*graphics, *scene, *cam);
			rendererGBuffer->render(*graphics, *scene, *cam);
			//rendererSSAO->render(*graphics, *scene, *cam);
			//rendererEnvLighting->render(*graphics, *scene, *cam);
			//rendererTransparency->render(*graphics, *scene, *cam);
			//rendererLocalLighting->render(*graphics, *scene, *cam);
			//computerPtLight->compute(*graphics, *scene, *cam);
			//computerHDR->compute(*graphics);
			//computerBloom->compute(*graphics);
			//rendererTonemap->render(*graphics);
			rendererWireframe->render(*graphics, *cam, *scene);
		}

		executor.run(taskflow).wait();
		// to wait for all threads to finish before execute
		executeCommandList();

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
	delete computerHDR;
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