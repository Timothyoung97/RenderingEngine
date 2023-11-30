#define SDL_MAIN_HANDLED

#include "portable-file-dialogs.h"

//Custom Header
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

#include "engine.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>  
#include <crtdbg.h>

using namespace DirectX;

tre::Engine* pEngine;

int main()
{
	_CrtMemState s1;
	_CrtMemState s2;
	_CrtMemState s3;

	_CrtMemCheckpoint(&s1);

	{
		tre::Engine e;
		pEngine = &e;
		e.init();

		// Crate Global Resource
		srand((uint32_t)time(NULL));																	// set random seed
		tre::Scene scene(e.device->device.Get(), e.device->context.Get()); 								// Create Scene
		tre::Camera cam(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);											// Create Camera

		// Loading Models
		std::string basePathStr = tre::Utility::getBasePathStr();										// File path
		pfd::open_file f = pfd::open_file("Choose files to read", basePathStr,
			{
				"All Files", "*" ,
				"glTF Files (.gltf)", "*.gltf",
				"obj Files (.obj)", "*.obj",
			},
			pfd::opt::force_path
		);

		tre::ModelLoader ml;
		if (f.result().size()) {
			ml.load(e.device->device.Get(), f.result()[0]);

			for (int i = 0; i < ml._objectWithMesh.size(); i++) {
				tre::Object* pObj = ml._objectWithMesh[i];
				scene._pObjQ.push_back(pObj);
			}
		}

		//Create Renderer
		tre::Graphics graphics(e.device->device.Get(), e.device->context.Get(), e.window->getWindowHandle());
		tre::RendererCSM rendererCSM(e.device->device.Get(), e.device->context.Get());
		tre::RendererEnvironmentLighting rendererEnvLighting(e.device->device.Get(), e.device->context.Get());
		tre::RendererGBuffer rendererGBuffer(e.device->device.Get(), e.device->context.Get());
		tre::RendererHDR rendererHDR(e.device->device.Get(), e.device->context.Get());
		tre::RendererLocalLighting rendererLocalLighting(e.device->device.Get(), e.device->context.Get());
		tre::RendererSSAO rendererSSAO(e.device->device.Get(), e.device->context.Get());
		tre::RendererTransparency rendererTransparency(e.device->device.Get(), e.device->context.Get());
		tre::RendererWireframe rendererWireframe(e.device->device.Get(), e.device->context.Get());

		// Create Computer 
		tre::ComputerPointLight computerPtLight(e.device->device.Get(), e.device->context.Get());

		// Input Handler
		tre::Input input;
		tre::Control control;

		// Delta Time between frame
		float deltaTime = 0;

		// Testing Obj
		{
			tre::Object debugModel;	
			debugModel.pObjMeshes = { &scene._debugMeshes[4] };
			debugModel.pObjMeshes[0]->pMaterial = &scene._debugMaterials[4];
			debugModel.objPos = XMFLOAT3(.0f, .5f, .0f);
			debugModel.objScale = XMFLOAT3(1.f, 1.f, 1.f);
			debugModel.objRotation = XMFLOAT3(.0f, .0f, .0f);
			debugModel.ritterBs = { debugModel.pObjMeshes[0]->ritterSphere };
			debugModel.naiveBs = { debugModel.pObjMeshes[0]->naiveSphere };
			debugModel.aabb = { debugModel.pObjMeshes[0]->aabb };
			debugModel._boundingVolumeColor = { tre::colorF(Colors::LightGreen) };
			debugModel.isInView = { true };
			scene._objQ.push_back(debugModel);
			scene._pObjQ.push_back(&scene._objQ.back());
		}
		tre::Object* pDebugModel = scene._pObjQ.back();

		// Stats Update
		for (int i = 0; i < scene._pObjQ.size(); i++) {
			for (int j = 0; j < scene._pObjQ[i]->pObjMeshes.size(); j++) {
				graphics.stats.totalMeshCount++;
				tre::Mesh* pMesh = scene._pObjQ[i]->pObjMeshes[j];
				if ((pMesh->pMaterial->objTexture != nullptr && pMesh->pMaterial->objTexture->hasAlphaChannel)
					|| (pMesh->pMaterial->objTexture == nullptr && pMesh->pMaterial->baseColor.w < 1.0f)) {
					graphics.stats.transparentMeshCount++;
				}
				else {
					graphics.stats.opaqueMeshCount++;
				}
			}
		}

		// create imgui
		ImguiHelper imguiHelper(e.device->device.Get(), e.device->context.Get(), e.window, &scene, &graphics.setting, &graphics.stats, &cam, pDebugModel);

		// main loop
		while (!input.shouldQuit())
		{
			MICROPROFILE_SCOPE_CSTR("Frame");

			tre::Timer timer;
			graphics.clean();											// Clear buffer + clean up
			input.updateInputEvent();									// Update input event
			control.update(input, graphics, scene, cam, deltaTime);		// Update control
			cam.updateCamera();											// Update Camera
			computerPtLight.compute(graphics, scene, cam);				// Compute Pt Light's position
			scene.update(graphics, cam);								// Update Scene
			rendererCSM.render(graphics, scene, cam);					// CSM Shadow Pass
			rendererGBuffer.render(graphics, scene, cam);				// G-Buffer: Deferred normal, albedo and depth
			rendererSSAO.render(graphics, scene, cam);					// SSAO Pass
			rendererEnvLighting.render(graphics, scene, cam);			// Environment Lighting Pass
			rendererTransparency.render(graphics, scene, cam);			// Transparency Object Pass
			rendererLocalLighting.render(graphics, scene, cam);			// Local Lighting Pass
			rendererHDR.render(graphics);								// HDR Pass
			rendererWireframe.render(graphics, cam, scene);				// Wireframe Debug Pass
			imguiHelper.render();										// IMGUI tool
			graphics.present();											// Present final frame image
			timer.spinWait();											// framerate control
			deltaTime = timer.getDeltaTime();							// Update frame time
			e.profiler->recordFrame();										// record each frame
			e.profiler->storeToDisk(control.toDumpFile);					// Store profiel log into disk
		}

		// Clean up
		{
			imguiHelper.cleanup();
			e.close();
		}
	}

	_CrtMemCheckpoint(&s2);

	if (_CrtMemDifference(&s3, &s1, &s2))
		_CrtMemDumpStatistics(&s3);

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtDumpMemoryLeaks();

	return 0;
}