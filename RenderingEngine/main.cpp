#define SDL_MAIN_HANDLED

#include <dxgi1_4.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "spdlog/spdlog.h"
#include "portable-file-dialogs.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_dx11.h"

#include <algorithm>
#include <functional>
#include <array>

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
		}
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

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForD3D(window.window);
	ImGui_ImplDX11_Init(deviceAndContext.device.Get(), deviceAndContext.context.Get());

	// imgui setting
	bool show_demo_window = false;
	bool showBoundingVolume = false;
	bool pauseLight = false;
	bool ssaoSwitch = false;
	tre::BoundVolumeEnum typeOfBound = tre::AABBBoundingBox;
	int meshIdx = 0;
	float fovY = 45.0f;
	float ssaoSampleRadius = .1f, ssaoBias = .0f;
	bool csmDebugSwitch = false;
	int shadowCascadeOpaqueObjs[4] = { 0, 0, 0, 0 };
	int opaqueMeshCount = 0, transparentMeshCount = 0, totalMeshCount = 0;

	for (int i = 0; i < scene._pObjQ.size(); i++) {
		for (int j = 0; j < scene._pObjQ[i]->pObjMeshes.size(); j++) {
			totalMeshCount++;
			tre::Mesh* pMesh = scene._pObjQ[i]->pObjMeshes[j];
			if ((pMesh->material->objTexture != nullptr && pMesh->material->objTexture->hasAlphaChannel)
				|| (pMesh->material->objTexture == nullptr && pMesh->material->baseColor.w < 1.0f)) {
				transparentMeshCount++;
			} else {
				opaqueMeshCount++;
			}
		}
	}

	float planeIntervals[5] = {1.0f, 20.f, 100.f, 250.f, 500.f};
	XMFLOAT4 planeIntervalsF = { planeIntervals[1], planeIntervals[2], planeIntervals[3], planeIntervals[4]};

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

	// main loop
	while (!input.shouldQuit())
	{
		tre::Timer timer;

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

			totalMeshCount++;
			if ((newObj.pObjMeshes[0]->material->objTexture != nullptr && newObj.pObjMeshes[0]->material->objTexture->hasAlphaChannel)
				|| (newObj.pObjMeshes[0]->material->objTexture == nullptr && newObj.pObjMeshes[0]->material->baseColor.w < 1.0f)) {
				transparentMeshCount++;
			}
			else {
				opaqueMeshCount++;
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		if (true)
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Debug");

			ImGui::Checkbox("Demo Window", &show_demo_window);

			{	// SSAO
				ImGui::SeparatorText("SSAO");
				ImGui::Checkbox("SSAO Switch", &ssaoSwitch);
				ImGui::SliderFloat("SSAO Sample Radius", &ssaoSampleRadius, .000f, 2.f, "%.6f");
				ImGui::SliderFloat("SSAO Sample Bias", &ssaoBias, -.1f, .1f, "%.6f");
			}

			{	// Control for import models

				ImGui::SeparatorText("Test Object Control");
				
				float translation[3] = { pDebugModel->objPos.x, pDebugModel->objPos.y,  pDebugModel->objPos.z };
				ImGui::SliderFloat3("Translation", translation, .0f, 20.f);
				pDebugModel->objPos = XMFLOAT3(translation);

				float rotationXYZ[3] = { pDebugModel->objRotation.x, pDebugModel->objRotation.y, pDebugModel->objRotation.z};
				ImGui::SliderFloat3("Rotation", rotationXYZ, .0f, 360.f);
				pDebugModel->objRotation = XMFLOAT3(rotationXYZ);

				float scaleXYZ = pDebugModel->objScale.x;
				ImGui::SliderFloat("Scale", &scaleXYZ, .1f, 3.f);
				pDebugModel->objScale = XMFLOAT3(scaleXYZ, scaleXYZ, scaleXYZ);
				pDebugModel->_transformationFinal = tre::Maths::createTransformationMatrix(pDebugModel->objScale, pDebugModel->objRotation, pDebugModel->objPos);
			}

			{	// Camera Setting
				ImGui::SeparatorText("Camera");
				ImGui::SliderFloat("Camera FOV Y", &fovY, 1.0f, 179.0f);
				ImGui::SliderFloat("Camera Speed", &cam.cameraMoveSpeed, .0f, 1.0f);
			}

			{	// Bounding Type Selection
				ImGui::SeparatorText("Bounding Volume");
				ImGui::Checkbox("Show Bounding Volume", &showBoundingVolume);
				static int selectedIdx = 0;
				const char* names[] = { "AABB", "Ritter Sphere", "Naive Sphere" };

				if (ImGui::Button("Bounding Volume Type")) {
					ImGui::OpenPopup("boundVType");
				}
				ImGui::SameLine();
				ImGui::TextUnformatted(names[selectedIdx]);
				if (ImGui::BeginPopup("boundVType")) {
					ImGui::SeparatorText("Bounding Type");
					for (int i = 0; i < IM_ARRAYSIZE(names); i++)
						if (ImGui::Selectable(names[i])) {
							selectedIdx = i;
							switch (selectedIdx) {
							case 0:
								typeOfBound = tre::AABBBoundingBox;
								meshIdx = 0;
								break;
							case 1:
								typeOfBound = tre::RitterBoundingSphere;
								meshIdx = 1;
								break;
							case 2:
								typeOfBound = tre::NaiveBoundingSphere;
								meshIdx = 1;
								break;
							}
						}
					ImGui::EndPopup();
				}
			}

			{	// light 
				ImGui::SeparatorText("Lights");
				ImGui::Checkbox("Pause Light", &pauseLight);
				if (scene.lightResc.numOfLights < scene.lightResc.maxPointLightNum) {
					if (ImGui::Button("Add Pt Light")) {
						scene.lightResc.addRandPointLight();
					}
					ImGui::SameLine();
					ImGui::Text("Current Light Count: %d/%d", scene.lightResc.numOfLights, scene.lightResc.maxPointLightNum);
				} else {
					ImGui::Text("Max Light Count: %d/%d", scene.lightResc.numOfLights, scene.lightResc.maxPointLightNum);
				}

				ImGui::BulletText("Dir Light");
				ImGui::SliderFloat("Yaw", &scene.dirlightYaw, .0f, 360.f);
				ImGui::SliderFloat("Pitch", &scene.dirlightPitch, .0f, 89.f);
			}

			{	// farplane intervals
				ImGui::SeparatorText("Cascaded Shadow");
				ImGui::Text("Total Opaque Mesh: %d; Total Transparent Mesh: %d; All Mesh: %d;", opaqueMeshCount, transparentMeshCount, totalMeshCount);
				ImGui::Text("");
				ImGui::Checkbox("CSM Debug", &csmDebugSwitch);
				ImGui::Text("Opaque Draw in Shadow Cascade 0: %d / %d", shadowCascadeOpaqueObjs[0], opaqueMeshCount);
				ImGui::SliderFloat("Far Plane 0", &planeIntervalsF.x, planeIntervals[0], planeIntervals[2]);
				ImGui::Text("Opaque Draw in Shadow Cascade 1: %d / %d", shadowCascadeOpaqueObjs[1], opaqueMeshCount);
				ImGui::SliderFloat("Far Plane 1", &planeIntervalsF.y, planeIntervals[1], planeIntervals[3]);
				ImGui::Text("Opaque Draw in Shadow Cascade 2: %d / %d", shadowCascadeOpaqueObjs[2], opaqueMeshCount);
				ImGui::SliderFloat("Far Plane 2", &planeIntervalsF.z, planeIntervals[2], planeIntervals[4]);
				ImGui::Text("Opaque Draw in Shadow Cascade 3: %d / %d", shadowCascadeOpaqueObjs[3], opaqueMeshCount);
				ImGui::SliderFloat("Far Plane 3", &planeIntervalsF.w, planeIntervals[3], 1000.0f);

				planeIntervals[1] = planeIntervalsF.x, planeIntervals[2] = planeIntervalsF.y, planeIntervals[3] = planeIntervalsF.z, planeIntervals[4] = planeIntervalsF.w;
			}

			{	// Stats
				ImGui::SeparatorText("Debug Info");
				ImGui::Text("Within Frustcum/Total: %d / %d", scene._culledOpaqueObjQ.size() + scene._culledTransparentObjQ.size(), scene._pObjQ.size());
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			}

			ImGui::End();
		}

		// render clear context
		renderer.reset();

		// Update Camera
		cam.camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, .1f, 100.f);
		cam.updateCamera();

		// Update Bounding volume for all objects once
		scene.updateBoundingVolume(typeOfBound);

		renderer.clearShadowBuffer();

		renderer.configureStates(tre::RENDER_MODE::SHADOW_M);

		// shadow draw
		std::vector<XMMATRIX> lightViewProjs;
		for (int i = 0; i < 4; i++) { // for 4 quads

			// projection matrix of camera with specific near and far plane
			XMMATRIX projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, planeIntervals[i], planeIntervals[i + 1]);

			std::vector<XMVECTOR> corners = tre::Maths::getFrustumCornersWorldSpace(XMMatrixMultiply(cam.camView, projMatrix));

			XMVECTOR center = tre::Maths::getAverageVector(corners);

			XMMATRIX lightView = XMMatrixLookAtLH(center + XMVECTOR{ scene.dirlight.direction.x, scene.dirlight.direction.y, scene.dirlight.direction.z }, center, XMVECTOR{ .0f, 1.f, .0f });

			XMMATRIX lightOrthoProj = tre::Maths::createOrthoMatrixFromFrustumCorners(10.f, corners, lightView);

			XMMATRIX lightViewProj = XMMatrixMultiply(lightView, lightOrthoProj);

			lightViewProjs.push_back(lightViewProj);
		}

		for (int i = 0; i < 4; i++) {
			renderer.setShadowBufferDrawSection(i);

			// set const buffer from the light pov 
			tre::ConstantBuffer::setCamConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cam.camPositionV, lightViewProjs[i], lightViewProjs, planeIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), csmDebugSwitch, ssaoSwitch);

			tre::Frustum lightFrustum = tre::Maths::createFrustumFromViewProjectionMatrix(lightViewProjs[i]);

			scene.cullObject(lightFrustum, typeOfBound);
			shadowCascadeOpaqueObjs[i] = scene._culledOpaqueObjQ.size();

			// draw shadow only for opaque objects
			renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::SHADOW_M);
		}

		// culling for scene draw
		scene.cullObject(cam.cameraFrustum, typeOfBound);
		scene.updateTransparentQ(cam);

		// set const buffer for camera
		tre::ConstantBuffer::setCamConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cam.camPositionV, cam.camViewProjection, lightViewProjs, planeIntervalsF, scene.dirlight, scene.lightResc.numOfLights, XMFLOAT2(4096, 4096), csmDebugSwitch, ssaoSwitch);

		// using compute shader update lights
		deviceAndContext.context.Get()->CSSetShader(scene.lightResc.computeShaderPtLightMovement.pShader.Get(), NULL, 0u);
		deviceAndContext.context.Get()->CSSetUnorderedAccessViews(0, 1, scene.lightResc.pLightUnorderedAccessView.GetAddressOf(), nullptr);
		deviceAndContext.context.Get()->Dispatch(tre::Maths::divideAndRoundUp(scene.lightResc.numOfLights, 4u), 1u, 1u);
		deviceAndContext.context.Get()->CSSetUnorderedAccessViews(0, 1, scene.lightResc.nullUAV, nullptr);

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

		// 1st pass deferred normal & albedo
		renderer.configureStates(tre::RENDER_MODE::DEFERRED_OPAQUE_M);
		renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::DEFERRED_OPAQUE_M);

		renderer.clearSwapChainBuffer();

		// ssao pass
		if (ssaoSwitch) {
			tre::ConstantBuffer::setSSAOKernalConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), renderer._ssao.ssaoKernalSamples, ssaoSampleRadius, ssaoBias);
			renderer.fullscreenPass(tre::RENDER_MODE::SSAO_FULLSCREEN_PASS);
			renderer.fullscreenPass(tre::RENDER_MODE::SSAO_BLURRING_PASS);
		}

		// 2nd pass deferred lighting 
		renderer.fullscreenPass(tre::RENDER_MODE::DEFERRED_OPAQUE_LIGHTING_ENV_M);

		// Draw all transparent objects
		renderer.draw(scene._culledTransparentObjQ, tre::RENDER_MODE::TRANSPARENT_M);

		// Draw all deferred lighting volume
		renderer.deferredLightingLocalDraw(scene._wireframeObjQ, cam.camPositionV);

		// Draw debug
		if (showBoundingVolume) {
			renderer.draw(scene._wireframeObjQ, tre::RENDER_MODE::WIREFRAME_M);
			renderer.debugDraw(scene._culledOpaqueObjQ, scene._debugMeshes[meshIdx], typeOfBound, tre::RENDER_MODE::WIREFRAME_M);
			renderer.debugDraw(scene._culledTransparentObjQ, scene._debugMeshes[meshIdx], typeOfBound, tre::RENDER_MODE::WIREFRAME_M);
		}

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		CHECK_DX_ERROR(renderer._swapchain.mainSwapchain->Present( 0, 0) );

		scene.updateDirLight();

		while (timer.getDeltaTime() < 1000.0 / 60) {
		}

		deltaTime = timer.getDeltaTime();
	}

	//Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
