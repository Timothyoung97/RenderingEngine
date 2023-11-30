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

		if (f.result().size()) {
			e.ml->load(e.device->device.Get(), f.result()[0]);

			for (int i = 0; i < e.ml->_objectWithMesh.size(); i++) {
				tre::Object* pObj = e.ml->_objectWithMesh[i];
				e.scene->_pObjQ.push_back(pObj);
			}
		}

		// Stats Update
		for (int i = 0; i < e.scene->_pObjQ.size(); i++) {
			for (int j = 0; j < e.scene->_pObjQ[i]->pObjMeshes.size(); j++) {
				e.graphics->stats.totalMeshCount++;
				tre::Mesh* pMesh = e.scene->_pObjQ[i]->pObjMeshes[j];
				if ((pMesh->pMaterial->objTexture != nullptr && pMesh->pMaterial->objTexture->hasAlphaChannel)
					|| (pMesh->pMaterial->objTexture == nullptr && pMesh->pMaterial->baseColor.w < 1.0f)) {
					e.graphics->stats.transparentMeshCount++;
				}
				else {
					e.graphics->stats.opaqueMeshCount++;
				}
			}
		}

		// Delta Time between frame
		float deltaTime = 0;

		// main loop
		while (!e.input->shouldQuit())
		{
			MICROPROFILE_SCOPE_CSTR("Frame");

			tre::Timer timer;
			e.graphics->clean();											
			e.input->updateInputEvent();									
			e.control->update(*e.input, *e.graphics, *e.scene, *e.cam, deltaTime);		
			e.cam->updateCamera();											
			e.computerPtLight->compute(*e.graphics, *e.scene, *e.cam);				
			e.scene->update(*e.graphics, *e.cam);								
			e.rendererCSM->render(*e.graphics, *e.scene, *e.cam);					
			e.rendererGBuffer->render(*e.graphics, *e.scene, *e.cam);	
			e.rendererSSAO->render(*e.graphics, *e.scene, *e.cam);			
			e.rendererEnvLighting->render(*e.graphics, *e.scene, *e.cam);		
			e.rendererTransparency->render(*e.graphics, *e.scene, *e.cam);			
			e.rendererLocalLighting->render(*e.graphics, *e.scene, *e.cam);		
			e.rendererHDR->render(*e.graphics);							
			e.rendererWireframe->render(*e.graphics, *e.cam, *e.scene);			
			e.imguihelper->render();
			e.graphics->present();										
			timer.spinWait();											
			deltaTime = timer.getDeltaTime();							
			e.profiler->recordFrame();										
			e.profiler->storeToDisk(e.control->toDumpFile);				
		}

		// Clean up
		{
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