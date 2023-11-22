#define SDL_MAIN_HANDLED

#include <dxgi1_4.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "spdlog/spdlog.h"
#include "portable-file-dialogs.h"

#include <algorithm>
#include <functional>
#include <array>
#include <format>

//Custom Header
#include "timer.h"
#include "window.h"
#include "input.h"
#include "dxdebug.h"
#include "device.h"
#include "mesh.h"
#include "utility.h"
#include "camera.h"
#include "constbuffer.h"
#include "texture.h"
#include "object.h"
#include "shader.h"
#include "graphics.h"
#include "colors.h"
#include "boundingvolume.h"
#include "maths.h"
#include "scene.h"
#include "modelloader.h"
#include "imguihelper.h"
#include "rendererWireframe.h"
#include "rendererHDR.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

int main()
{
	// set random seed
	srand((uint32_t)time(NULL));

	//Create Window
	tre::Window window("RenderingEngine", tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);

	//Create Device 
	tre::Device deviceAndContext;

	{	// microprofile setting
		MicroProfileOnThreadCreate("Main");
		MicroProfileSetEnableAllGroups(true);
		MicroProfileSetForceMetaCounters(true);

		MicroProfileGpuInitD3D11(deviceAndContext.device.Get(), deviceAndContext.context.Get());
		MICROPROFILE_GPU_SET_CONTEXT(deviceAndContext.context.Get(), MicroProfileGetGlobalGpuThreadLog());
		MicroProfileStartContextSwitchTrace();
	}

	// Scene
	tre::Scene scene(deviceAndContext.device.Get(), deviceAndContext.context.Get());
	scene.createFloor();
	scene.updateDirLight();

	//Create Camera
	tre::Camera cam(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);

	//File path
	std::string basePathStr = tre::Utility::getBasePathStr();
	
	// Loading model
	tre::ModelLoader ml;

	pfd::open_file f = pfd::open_file("Choose files to read", basePathStr,
		{ 
			"All Files", "*" ,
			"glTF Files (.gltf)", "*.gltf",
			"obj Files (.obj)", "*.obj",
		},
		pfd::opt::force_path
	);

	if (f.result().size()) {
		ml.load(deviceAndContext.device.Get(), f.result()[0]);

		for (int i = 0; i < ml._objectWithMesh.size(); i++) {
			tre::Object* pObj = ml._objectWithMesh[i];
			scene._pObjQ.push_back(pObj);
		}
	}

	//Create Renderer
	tre::Graphics graphics(deviceAndContext.device.Get(), deviceAndContext.context.Get(), window.getWindowHandle());
	tre::RendererWireframe rendererWireframe(deviceAndContext.device.Get(), deviceAndContext.context.Get());
	tre::RendererHDR rendererHDR(deviceAndContext.device.Get(), deviceAndContext.context.Get());

	//Input Handler
	tre::Input input;
	
	//Delta Time between frame
	float deltaTime = 0;

	for (int i = 0; i < scene._pObjQ.size(); i++) {
		for (int j = 0; j < scene._pObjQ[i]->pObjMeshes.size(); j++) {
			graphics.stats.totalMeshCount++;
			tre::Mesh* pMesh = scene._pObjQ[i]->pObjMeshes[j];
			if ((pMesh->pMaterial->objTexture != nullptr && pMesh->pMaterial->objTexture->hasAlphaChannel)
				|| (pMesh->pMaterial->objTexture == nullptr && pMesh->pMaterial->baseColor.w < 1.0f)) {
				graphics.stats.transparentMeshCount++;
			} else {
				graphics.stats.opaqueMeshCount++;
			}
		}
	}

	// Testing Obj
	tre::Object debugModel;

	debugModel.pObjMeshes = { &scene._debugMeshes[4] };
	debugModel.pObjMeshes[0]->pMaterial = &scene._debugMaterials[3];
	debugModel.objPos = XMFLOAT3(.0f, .5f, .0f);
	debugModel.objScale = XMFLOAT3(1.f, 1.f, 1.f);
	debugModel.objRotation = XMFLOAT3(.0f, .0f, .0f);
	debugModel.ritterBs = { debugModel.pObjMeshes[0]->ritterSphere };
	debugModel.naiveBs = { debugModel.pObjMeshes[0]->naiveSphere };
	debugModel.aabb = { debugModel.pObjMeshes[0]->aabb };
	debugModel._boundingVolumeColor = { tre::colorF(Colors::LightGreen) };
	scene._objQ.push_back(debugModel);
	scene._pObjQ.push_back(&scene._objQ.back());

	tre::Object* pDebugModel = scene._pObjQ.back();

	// create imgui
	ImguiHelper imguiHelper(
		deviceAndContext.device.Get(),
		deviceAndContext.context.Get(),
		&window,
		&scene,
		&graphics.setting,
		&graphics.stats,
		&cam,
		pDebugModel
	);

	bool toDumpFile = false;

	// main loop
	while (!input.shouldQuit())
	{
		tre::Timer timer;

		// Resource Clean Up
		{
			MICROPROFILE_SCOPE_CSTR("Clean Up");
			while (!graphics.bufferQueue.empty()) {
				ID3D11Buffer* currBuffer = graphics.bufferQueue.back();
				graphics.bufferQueue.pop_back();
				currBuffer->Release();
			}
		}

		MICROPROFILE_SCOPE_CSTR("Frame");

		// Update keyboard event
		input.updateInputEvent();

		// Control
		if (input._keyState[SDL_SCANCODE_W]) { // control camera movement
			cam.moveCamera(cam.directionV * deltaTime);
			scene._toRecalDistFromCam = true;
		}
		else if (input._keyState[SDL_SCANCODE_S]) {
			cam.moveCamera(-cam.directionV * deltaTime);
			scene._toRecalDistFromCam = true;
		}
		else if (input._keyState[SDL_SCANCODE_D]) {
			cam.moveCamera(-cam.camRightV * deltaTime);
			scene._toRecalDistFromCam = true;
		}
		else if (input._keyState[SDL_SCANCODE_A]) {
			cam.moveCamera(cam.camRightV * deltaTime);
			scene._toRecalDistFromCam = true;
		}
		else if (input._keyState[SDL_SCANCODE_Q]) {
			cam.moveCamera(cam.defaultUpV * deltaTime);
			scene._toRecalDistFromCam = true;
		}
		else if (input._keyState[SDL_SCANCODE_E]) {
			cam.moveCamera(-cam.defaultUpV * deltaTime);
			scene._toRecalDistFromCam = true;
		}
		else if (input._mouseWheelScollY != 0) {
			cam.cameraMoveSpeed = SDL_clamp(cam.cameraMoveSpeed + input._mouseWheelScollY, .001f, .5f);
		}
		else if (input._mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) { // control camera angle
			cam.turnCamera(input._deltaDisplacement.x, input._deltaDisplacement.y);
		}
		else if (input._keyState[SDL_SCANCODE_SPACE]) {
			for (int i = 0; i < 100; i++) {
				// Create new obj
				tre::Object* pNewObj = scene.addRandomObj();

				graphics.stats.totalMeshCount++;
				if ((pNewObj->pObjMeshes[0]->pMaterial->objTexture != nullptr && pNewObj->pObjMeshes[0]->pMaterial->objTexture->hasAlphaChannel)
					|| (pNewObj->pObjMeshes[0]->pMaterial->objTexture == nullptr && pNewObj->pObjMeshes[0]->pMaterial->baseColor.w < 1.0f)) {
					graphics.stats.transparentMeshCount++;
				}
				else {
					graphics.stats.opaqueMeshCount++;
				}
			}
		}
		else if (input._keyState[SDL_SCANCODE_K]) {
			toDumpFile = true;
		}

		// render clear context
		graphics.reset();

		// Update Camera
		cam.camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(cam.fovY), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, .1f, 250.f);
		cam.updateCamera();

		{	// Update Bounding volume for all objects once
			MICROPROFILE_SCOPE_CSTR("Update Bounding Volume");
			scene.updateBoundingVolume(graphics.setting.typeOfBound);
		}

		// Shadow Draw
		std::vector<XMMATRIX> lightViewProjs;
		for (int i = 0; i < 4; i++) { // for 4 quads

			MICROPROFILE_SCOPE_CSTR("Build CSM View Projection Matrix");

			// projection matrix of camera with specific near and far plane
			XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, graphics.setting.csmPlaneIntervals[i], graphics.setting.csmPlaneIntervals[i + 1]);

			std::vector<XMVECTOR> corners = tre::Maths::getFrustumCornersWorldSpace(XMMatrixMultiply(cam.camView, projMatrix));

			XMVECTOR center = tre::Maths::getAverageVector(corners);

			XMMATRIX lightView = XMMatrixLookAtLH(center + XMVECTOR{ scene.dirlight.direction.x, scene.dirlight.direction.y, scene.dirlight.direction.z }, center, XMVECTOR{ .0f, 1.f, .0f });

			XMMATRIX lightOrthoProj = tre::Maths::createOrthoMatrixFromFrustumCorners(10.f, corners, lightView);

			XMMATRIX lightViewProj = XMMatrixMultiply(lightView, lightOrthoProj);

			lightViewProjs.push_back(lightViewProj);
		}

		ID3D11Buffer* constBufferCSMViewProj = tre::ConstantBuffer::createConstBuffer(deviceAndContext.device.Get(), (UINT)sizeof(tre::ViewProjectionStruct));
		{
			PROFILE_GPU_SCOPED("CSM Quad Draw");

			for (int i = 0; i < 4; i++) {

				{	// Culling based on pointlight's view projection matrix
					MICROPROFILE_SCOPE_CSTR("CSM Quad Obj Culling");

					graphics.setShadowBufferDrawSection(i);

					// Const Buffer 
					{
						// Create struct info and submit data to const buffer
						tre::ViewProjectionStruct csmViewProjStruct = tre::ConstantBuffer::createViewProjectionStruct(lightViewProjs[i]);
						tre::ConstantBuffer::updateConstBufferData(deviceAndContext.context.Get(), constBufferCSMViewProj, &csmViewProjStruct, (UINT)sizeof(tre::ViewProjectionStruct));

						// Binding 
						deviceAndContext.context.Get()->VSSetConstantBuffers(0u, 1u, &constBufferCSMViewProj);
					}

					tre::Frustum lightFrustum = tre::Maths::createFrustumFromViewProjectionMatrix(lightViewProjs[i]);

					scene.cullObject(lightFrustum, graphics.setting.typeOfBound);
					{
						MICROPROFILE_SCOPE_CSTR("Grouping instances (Opaque only)");
						scene.updateCulledOpaqueQ();
					}

					graphics.stats.shadowCascadeOpaqueObjs[i] = scene._culledOpaqueObjQ.size();
				}

				// draw shadow only for opaque objects
				//renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::SHADOW_M); // non instanced
				graphics.instancedDraw(scene._culledOpaqueObjQ, tre::RENDER_MODE::INSTANCED_SHADOW_M); // instanced
			}
		}

		{	// culling for scene draw
			MICROPROFILE_SCOPE_CSTR("Scene Obj Culling");
			scene.cullObject(cam.cameraFrustum, graphics.setting.typeOfBound);
			{
				MICROPROFILE_SCOPE_CSTR("Grouping instances (Opaque + Transparent)");
				scene.updateCulledOpaqueQ();
				scene.updateCulledTransparentQ(cam);
			}
		}

		// 1st pass deferred normal & albedo
		ID3D11Buffer* constBufferCamViewProj = tre::ConstantBuffer::createConstBuffer(deviceAndContext.device.Get(), (UINT)sizeof(tre::ViewProjectionStruct));
		{
			PROFILE_GPU_SCOPED("G-Buffer");
			//renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::DEFERRED_OPAQUE_M); // non instanced

			// Update const buffer and binding
			{
				tre::ViewProjectionStruct vpStruct = tre::ConstantBuffer::createViewProjectionStruct(cam.camViewProjection);
				tre::ConstantBuffer::updateConstBufferData(deviceAndContext.context.Get(), constBufferCamViewProj, &vpStruct, (UINT)sizeof(tre::ViewProjectionStruct));

				deviceAndContext.context.Get()->VSSetConstantBuffers(0u, 1u, &constBufferCamViewProj);
			}

			graphics.instancedDraw(scene._culledOpaqueObjQ, tre::RENDER_MODE::INSTANCED_DEFERRED_OPAQUE_M); // instanced
		}

		// set const buffer for global info
		ID3D11Buffer* constBufferGlobalInfo = tre::ConstantBuffer::createConstBuffer(deviceAndContext.device.Get(), (UINT)sizeof(tre::GlobalInfoStruct));
		{
			// Create struct info and submit data to constant buffer
			tre::GlobalInfoStruct globalInfoStruct = tre::ConstantBuffer::createGlobalInfoStruct(cam.camPositionV, cam.camViewProjection, lightViewProjs, graphics.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), graphics.setting.csmDebugSwitch, graphics.setting.ssaoSwitch);
			tre::ConstantBuffer::updateConstBufferData(deviceAndContext.context.Get(), constBufferGlobalInfo, &globalInfoStruct, (UINT)sizeof(tre::GlobalInfoStruct));

			// Bind to shaders
			deviceAndContext.context.Get()->VSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
			deviceAndContext.context.Get()->PSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
			deviceAndContext.context.Get()->CSSetConstantBuffers(0u, 1u, &constBufferGlobalInfo);
		}

		// using compute shader update lights
		scene.lightResc.dispatch();

		{
			MICROPROFILE_SCOPE_CSTR("CPU Point Light Update");
			PROFILE_GPU_SCOPED("CPU Point Light Update");

			scene.lightResc.updatePtLightCPU();
			scene._pointLightObjQ.clear();
			scene._pointLightObjQ.reserve(scene.lightResc.readOnlyPointLightQ.size());
			scene._wireframeObjQ.clear();

			for (int i = 0; i < scene.lightResc.readOnlyPointLightQ.size(); i++) {
				tre::Object newLightObj;

				newLightObj.pObjMeshes = { &scene._debugMeshes[1] }; // sphere
				newLightObj.pObjMeshes[0]->pMaterial = &scene._debugMaterials[2];
				newLightObj.objPos = scene.lightResc.readOnlyPointLightQ[i].pos;
				newLightObj.objScale = XMFLOAT3(scene.lightResc.readOnlyPointLightQ[i].range, scene.lightResc.readOnlyPointLightQ[i].range, scene.lightResc.readOnlyPointLightQ[i].range);
				newLightObj.objRotation = XMFLOAT3(.0f, .0f, .0f);
				newLightObj._boundingVolumeColor = { tre::colorF(Colors::White) };
				newLightObj._transformationFinal = tre::Maths::createTransformationMatrix(newLightObj.objScale, newLightObj.objRotation, newLightObj.objPos);
				newLightObj._boundingVolumeTransformation = newLightObj._transformationFinal;

				scene._pointLightObjQ.push_back(newLightObj);
				scene._wireframeObjQ.push_back(std::make_pair(&scene._pointLightObjQ.back(), scene._pointLightObjQ.back().pObjMeshes[0]));
			}
			deviceAndContext.context.Get()->PSSetShaderResources(2, 1, scene.lightResc.pLightShaderRescView.GetAddressOf());
		}

		// ssao pass
		ID3D11Buffer* constBufferSSAOKernal = tre::ConstantBuffer::createConstBuffer(deviceAndContext.device.Get(), (UINT)sizeof(tre::SSAOKernalStruct));;
		if (graphics.setting.ssaoSwitch) {

			// SSAO const buffer creation and binding
			{
				tre::SSAOKernalStruct ssaoKernalStruct = tre::ConstantBuffer::createSSAOKernalStruct(graphics._ssao.ssaoKernalSamples, graphics.setting.ssaoSampleRadius);
				tre::ConstantBuffer::updateConstBufferData(deviceAndContext.context.Get(), constBufferSSAOKernal, &ssaoKernalStruct, (UINT)sizeof(tre::SSAOKernalStruct));

				deviceAndContext.context.Get()->PSSetConstantBuffers(3u, 1u, &constBufferSSAOKernal);
			}

			PROFILE_GPU_SCOPED("SSAO Pass");
			graphics.fullscreenPass(tre::RENDER_MODE::SSAO_FULLSCREEN_PASS);
			graphics.fullscreenPass(tre::RENDER_MODE::SSAO_BLURRING_PASS);
		}

		// 2nd pass deferred lighting
		{
			PROFILE_GPU_SCOPED("Environmental Lighting");
			graphics.fullscreenPass(tre::RENDER_MODE::DEFERRED_OPAQUE_LIGHTING_ENV_M);
		}

		// Draw all transparent objects
		{
			PROFILE_GPU_SCOPED("Transparent Obj");
			graphics.draw(scene._culledTransparentObjQ, tre::RENDER_MODE::TRANSPARENT_M); // here can draw instanced
		}

		// Draw all deferred lighting volume
		{
			deviceAndContext.context.Get()->PSGetShaderResources(2u, 1u, scene.lightResc.pLightShaderRescView.GetAddressOf());
			PROFILE_GPU_SCOPED("Local Lighting");
			graphics.deferredLightingLocalDraw(scene._wireframeObjQ, cam.camPositionV);
		}

		deviceAndContext.context.Get()->OMSetRenderTargets(0, nullptr, nullptr);

		// Luminance Histogram
		{
			PROFILE_GPU_SCOPED("CS: Luminance Histogram");
			rendererHDR.setConstBufferLuminSetting(graphics);
			rendererHDR.dispatchHistogram(graphics);
			rendererHDR.dispatchAverage(graphics);
		}

		// HDR full screen pass
		{
			PROFILE_GPU_SCOPED("HDR");
			rendererHDR.setConstBufferHDR(graphics);
			rendererHDR.fullscreenPass(graphics);
		}

		// Wireframe draw
		{
			PROFILE_GPU_SCOPED("Bounding Volume Wireframe");
			// Bind Camera View Projection Constant Buffer
			{
				deviceAndContext.context.Get()->VSSetConstantBuffers(0u, 1u, &constBufferCamViewProj);
			}

			rendererWireframe.drawInstanced(&graphics, scene._wireframeObjQ);			// for point lights
			rendererWireframe.drawInstanced(&graphics, scene._culledOpaqueObjQ);		// for opaque objects
			rendererWireframe.drawInstanced(&graphics, scene._culledTransparentObjQ);	// for transparent objects
		}

		{
			MICROPROFILE_SCOPE_CSTR("IMGUI");
			PROFILE_GPU_SCOPED("IMGUI");
			imguiHelper.render();
		}

		{
			MICROPROFILE_SCOPE_CSTR("Swap Chain Present");

			const UINT kSyncInterval = 0; // Need to sync CPU frame to this function if we want V-SYNC

			// When using sync interval 0, it is recommended to always pass the tearing flag when it is supported.
			const UINT presentFlags = (kSyncInterval == 0 && graphics._swapchain.m_bTearingSupported) ? DXGI_PRESENT_ALLOW_TEARING : 0;

			CHECK_DX_ERROR(graphics._swapchain.mainSwapchain->Present(kSyncInterval, presentFlags));
		}

		scene.updateDirLight();

		if (toDumpFile) {
			spdlog::info("Profiling");
			MicroProfileDumpFileImmediately("profile.html", nullptr, deviceAndContext.context.Get());
			toDumpFile = false;
		}

		{
			MICROPROFILE_SCOPE_CSTR("Spin Wait");
			// framerate control
			while (timer.getDeltaTime() < 1000.0 / 60) {
			}
		}

		deltaTime = timer.getDeltaTime();

		// clean up per frame resource
		{
			MICROPROFILE_SCOPE_CSTR("Clean Up");
			constBufferGlobalInfo->Release();
			constBufferCSMViewProj->Release();
			constBufferCamViewProj->Release();
			constBufferSSAOKernal->Release();
		}

		// record each frame
		MicroProfileFlip(deviceAndContext.context.Get());
	}

	MicroProfileGpuShutdown();
	MicroProfileShutdown();
	imguiHelper.cleanup();

	return 0;
}