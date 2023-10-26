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
#include "renderer.h"
#include "colors.h"
#include "boundingvolume.h"
#include "maths.h"
#include "scene.h"
#include "modelloader.h"
#include "imguihelper.h"

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
	tre::Renderer renderer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), window.getWindowHandle());

	//Input Handler
	tre::Input input;
	
	//Delta Time between frame
	float deltaTime = 0;

	for (int i = 0; i < scene._pObjQ.size(); i++) {
		for (int j = 0; j < scene._pObjQ[i]->pObjMeshes.size(); j++) {
			renderer.stats.totalMeshCount++;
			tre::Mesh* pMesh = scene._pObjQ[i]->pObjMeshes[j];
			if ((pMesh->material->objTexture != nullptr && pMesh->material->objTexture->hasAlphaChannel)
				|| (pMesh->material->objTexture == nullptr && pMesh->material->baseColor.w < 1.0f)) {
				renderer.stats.transparentMeshCount++;
			} else {
				renderer.stats.opaqueMeshCount++;
			}
		}
	}

	// Testing Obj
	tre::Object debugModel;

	debugModel.pObjMeshes = { &scene._debugMeshes[4] };
	debugModel.pObjMeshes[0]->material = &scene._debugMaterials[3];
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
		&renderer.setting,
		&renderer.stats,
		&cam,
		pDebugModel
	);

	bool toDumpFile = false;

	// main loop
	while (!input.shouldQuit())
	{
		tre::Timer timer;

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
			// Create new obj
			tre::Object newObj;

			float scaleVal = tre::Utility::getRandomFloat(3);
			int textureIdx = tre::Utility::getRandomInt(1);

			int selectIdx = tre::Utility::getRandomInt(1);
			newObj.pObjMeshes = { &scene._debugMeshes[5 + selectIdx] };
			newObj.pObjMeshes[0]->material = &scene._debugMaterials[selectIdx];
			newObj.objPos = XMFLOAT3(tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20));
			newObj.objScale = XMFLOAT3(scaleVal, scaleVal, scaleVal);
			newObj.objRotation = XMFLOAT3(tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360));
			newObj._boundingVolumeColor = { tre::colorF(Colors::Green) };
			newObj.ritterBs = { newObj.pObjMeshes[0]->ritterSphere };
			newObj.naiveBs = { newObj.pObjMeshes[0]->naiveSphere };
			newObj.aabb = { newObj.pObjMeshes[0]->aabb };
			newObj._transformationFinal = tre::Maths::createTransformationMatrix(newObj.objScale, newObj.objRotation, newObj.objPos);

			scene._objQ.push_back(newObj);
			scene._pObjQ.push_back(&scene._objQ.back());

			renderer.stats.totalMeshCount++;
			if ((newObj.pObjMeshes[0]->material->objTexture != nullptr && newObj.pObjMeshes[0]->material->objTexture->hasAlphaChannel)
				|| (newObj.pObjMeshes[0]->material->objTexture == nullptr && newObj.pObjMeshes[0]->material->baseColor.w < 1.0f)) {
				renderer.stats.transparentMeshCount++;
			}
			else {
				renderer.stats.opaqueMeshCount++;
			}
		}
		else if (input._keyState[SDL_SCANCODE_K]) {
			toDumpFile = true;
		}

		// render clear context
		renderer.reset();

		// Update Camera
		cam.camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(cam.fovY), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, .1f, 250.f);
		cam.updateCamera();

		{	// Update Bounding volume for all objects once
			MICROPROFILE_SCOPE_CSTR("Update Bounding Volume");
			scene.updateBoundingVolume(renderer.setting.typeOfBound);
		}

		renderer.configureStates(tre::RENDER_MODE::SHADOW_M);

		// Shadow Draw
		std::vector<XMMATRIX> lightViewProjs;
		for (int i = 0; i < 4; i++) { // for 4 quads

			MICROPROFILE_SCOPE_CSTR("Build CSM View Projection Matrix");

			// projection matrix of camera with specific near and far plane
			XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, renderer.setting.csmPlaneIntervals[i], renderer.setting.csmPlaneIntervals[i + 1]);

			std::vector<XMVECTOR> corners = tre::Maths::getFrustumCornersWorldSpace(XMMatrixMultiply(cam.camView, projMatrix));

			XMVECTOR center = tre::Maths::getAverageVector(corners);

			XMMATRIX lightView = XMMatrixLookAtLH(center + XMVECTOR{ scene.dirlight.direction.x, scene.dirlight.direction.y, scene.dirlight.direction.z }, center, XMVECTOR{ .0f, 1.f, .0f });

			XMMATRIX lightOrthoProj = tre::Maths::createOrthoMatrixFromFrustumCorners(10.f, corners, lightView);

			XMMATRIX lightViewProj = XMMatrixMultiply(lightView, lightOrthoProj);

			lightViewProjs.push_back(lightViewProj);
		}

		{
			PROFILE_GPU_SCOPED("CSM Quad Draw");

			for (int i = 0; i < 4; i++) {

				{	// Culling based on pointlight's view projection matrix
					MICROPROFILE_SCOPE_CSTR("CSM Quad Obj Culling");

					renderer.setShadowBufferDrawSection(i);

					// set const buffer from the light pov 
					tre::ConstantBuffer::setCamConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cam.camPositionV, lightViewProjs[i], lightViewProjs, renderer.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), renderer.setting.csmDebugSwitch, renderer.setting.ssaoSwitch);

					tre::Frustum lightFrustum = tre::Maths::createFrustumFromViewProjectionMatrix(lightViewProjs[i]);

					scene.cullObject(lightFrustum, renderer.setting.typeOfBound);
					renderer.stats.shadowCascadeOpaqueObjs[i] = scene._culledOpaqueObjQ.size();
				}

				// draw shadow only for opaque objects
				renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::SHADOW_M);
			}
		}

		{	// culling for scene draw
			MICROPROFILE_SCOPE_CSTR("Scene Obj Culling");
			scene.cullObject(cam.cameraFrustum, renderer.setting.typeOfBound);
			scene.updateTransparentQ(cam);
		}

		// set const buffer for camera
		tre::ConstantBuffer::setCamConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cam.camPositionV, cam.camViewProjection, lightViewProjs, renderer.setting.csmPlaneIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), renderer.setting.csmDebugSwitch, renderer.setting.ssaoSwitch);

		// using compute shader update lights
		scene.lightResc.dispatch();

		{
			MICROPROFILE_SCOPE_CSTR("CPU Point Light Update");
			PROFILE_GPU_SCOPED("CPU Point Light Update");

			scene.lightResc.updatePtLightCPU();
			scene._pointLightObjQ.clear();
			scene._wireframeObjQ.clear();

			for (int i = 0; i < scene.lightResc.readOnlyPointLightQ.size(); i++) {
				tre::Object newLightObj;

				newLightObj.pObjMeshes = { &scene._debugMeshes[1] }; // sphere
				newLightObj.pObjMeshes[0]->material = &scene._debugMaterials[2];
				newLightObj.objPos = scene.lightResc.readOnlyPointLightQ[i].pos;
				newLightObj.objScale = XMFLOAT3(scene.lightResc.readOnlyPointLightQ[i].range, scene.lightResc.readOnlyPointLightQ[i].range, scene.lightResc.readOnlyPointLightQ[i].range);
				newLightObj.objRotation = XMFLOAT3(.0f, .0f, .0f);
				newLightObj._boundingVolumeColor = { tre::colorF(Colors::White) };
				newLightObj._transformationFinal = tre::Maths::createTransformationMatrix(newLightObj.objScale, newLightObj.objRotation, newLightObj.objPos);

				scene._pointLightObjQ.push_back(newLightObj);
				scene._wireframeObjQ.push_back(std::make_pair(&scene._pointLightObjQ.back(), scene._pointLightObjQ.back().pObjMeshes[0]));
			}
			deviceAndContext.context.Get()->PSSetShaderResources(2, 1, scene.lightResc.pLightShaderRescView.GetAddressOf());
		}

		// 1st pass deferred normal & albedo
		{
			PROFILE_GPU_SCOPED("G-Buffer");
			renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::DEFERRED_OPAQUE_M);
		}

		// ssao pass
		if (renderer.setting.ssaoSwitch) {
			tre::ConstantBuffer::setSSAOKernalConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), renderer._ssao.ssaoKernalSamples, renderer.setting.ssaoSampleRadius);
			PROFILE_GPU_SCOPED("SSAO Pass");
			renderer.fullscreenPass(tre::RENDER_MODE::SSAO_FULLSCREEN_PASS);
			renderer.fullscreenPass(tre::RENDER_MODE::SSAO_BLURRING_PASS);
		}

		// 2nd pass deferred lighting
		{
			PROFILE_GPU_SCOPED("Environmental Lighting");
			renderer.fullscreenPass(tre::RENDER_MODE::DEFERRED_OPAQUE_LIGHTING_ENV_M);
		}

		// Draw all transparent objects
		{
			PROFILE_GPU_SCOPED("Transparent Obj");
			renderer.draw(scene._culledTransparentObjQ, tre::RENDER_MODE::TRANSPARENT_M);
		}

		// Draw all deferred lighting volume
		{
			deviceAndContext.context.Get()->PSGetShaderResources(2u, 1u, scene.lightResc.pLightShaderRescView.GetAddressOf());
			PROFILE_GPU_SCOPED("Local Lighting");
			renderer.deferredLightingLocalDraw(scene._wireframeObjQ, cam.camPositionV);
		}

		deviceAndContext.context.Get()->OMSetRenderTargets(0, nullptr, nullptr);

		tre::ConstantBuffer::setLuminaceConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), XMFLOAT2(1.0f, 20.f), 1.5f);

		renderer._hdrBuffer.dispatchHistogram();

		renderer._hdrBuffer.dispatchAverage();

		deviceAndContext.context.Get()->PSSetShaderResources(8, 1, renderer._hdrBuffer.pShaderResViewHdrTexture.GetAddressOf()); // normal

		tre::ConstantBuffer::setHDRConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), renderer.setting.exposure);
		// HDR
		{
			PROFILE_GPU_SCOPED("HDR");
			renderer.fullscreenPass(tre::RENDER_MODE::TONE_MAPPING_PASS);
		}

		// Draw debug
		if (renderer.setting.showBoundingVolume) {
			PROFILE_GPU_SCOPED("Bounding Volume Wireframe");
			renderer.draw(scene._wireframeObjQ, tre::RENDER_MODE::WIREFRAME_M);
			renderer.debugDraw(scene._culledOpaqueObjQ, scene._debugMeshes[renderer.setting.meshIdx], renderer.setting.typeOfBound, tre::RENDER_MODE::WIREFRAME_M);
			renderer.debugDraw(scene._culledTransparentObjQ, scene._debugMeshes[renderer.setting.meshIdx], renderer.setting.typeOfBound, tre::RENDER_MODE::WIREFRAME_M);
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
			const UINT presentFlags = (kSyncInterval == 0 && renderer._swapchain.m_bTearingSupported) ? DXGI_PRESENT_ALLOW_TEARING : 0;

			CHECK_DX_ERROR(renderer._swapchain.mainSwapchain->Present(kSyncInterval, presentFlags));
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

		// record each frame
		MicroProfileFlip(deviceAndContext.context.Get());
	}

	MicroProfileGpuShutdown();
	MicroProfileShutdown();
	imguiHelper.cleanup();

	return 0;
}