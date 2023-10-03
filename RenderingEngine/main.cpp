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
	tre::Scene scene(deviceAndContext.device.Get());
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
			"glTF Files (.gltf)", "*.gltf",
			"obj Files (.obj)", "*.obj",
			"All Files", "*" 
		}
	);

	ml.load(deviceAndContext.device.Get(), f.result()[0]);

	for (int i = 0; i < ml._objectWithMesh.size(); i++) {
		tre::Object* obj = ml._objectWithMesh[i];
		for (int j = 0; j < obj->pObjMeshes.size(); j++) {
			tre::Mesh* mesh = obj->pObjMeshes[j];
			if ((mesh->material->objTexture != nullptr && mesh->material->objTexture->hasAlphaChannel) ||
				(mesh->material->objTexture == nullptr && mesh->material->baseColor.w < 1.0f)) {

				obj->distFromCam = tre::Maths::distBetweentObjToCam(scene._objQ.back().objPos, cam.camPositionV);

				scene._transparentObjQ.push_back(std::make_pair(obj, mesh));
				
				scene._toSortTransparentQ = true;
			} else {
				scene._opaqueObjQ.push_back(std::make_pair(obj, mesh));
			}
		}
	}

	//Create Renderer
	tre::Renderer renderer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), window.getWindowHandle());

	//Input Handler
	tre::Input input;
	
	//Delta Time between frame
	float deltaTime = 0;

	float stackAnglePtLight[] = { .0f, .0f, .0f, .0f };
	float sectorAnglePtLight[] = { .0f, .0f, .0f, -90.0f };
	XMFLOAT3 originPtLight[] = { XMFLOAT3(3.0f, 3.0f, 3.0f),  XMFLOAT3(-3.0f, -3.0f, -3.0f), XMFLOAT3(.0f, .0f, .0f), XMFLOAT3(-1.0f, .0f, -1.0f) };

	scene.lightResc.updateBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get());

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
	tre::BoundVolumeEnum typeOfBound = tre::AABBBoundingBox;
	int meshIdx = 0;
	float fovY = 45.0f;
	bool csmDebugSwitch = false;

	float planeIntervals[5] = {1.0f, 20.f, 100.f, 250.f, 500.f};
	XMFLOAT4 planeIntervalsF = { planeIntervals[1], planeIntervals[2], planeIntervals[3], planeIntervals[4]};

	// Testing Obj
	tre::Object debugModel;

	debugModel.pObjMeshes = { &scene._debugMeshes[4] };
	debugModel.pObjMeshes[0]->material = &scene._debugMaterials[3];
	debugModel.objPos = XMFLOAT3(.0f, 1.0f, .0f);
	debugModel.objScale = XMFLOAT3(1.f, 1.f, 1.f);
	debugModel.objRotation = XMFLOAT3(.0f, .0f, .0f);
	debugModel.ritterBs = { debugModel.pObjMeshes[0]->ritterSphere };
	debugModel.naiveBs = { debugModel.pObjMeshes[0]->naiveSphere };
	debugModel.aabb = { debugModel.pObjMeshes[0]->aabb };
	debugModel._boundingVolumeColor = { tre::colorF(Colors::Green) };
	scene._objQ.push_back(debugModel);

	tre::Object* pDebugModel = &scene._objQ.back();

	// transparent queue -> object with texture with alpha channel or object with color.w below 1.0f
	if ((scene._objQ.back().pObjMeshes[0]->material->objTexture != nullptr && scene._objQ.back().pObjMeshes[0]->material->objTexture->hasAlphaChannel)
		|| (scene._objQ.back().pObjMeshes[0]->material->objTexture == nullptr && scene._objQ.back().pObjMeshes[0]->material->baseColor.w < 1.0f)) {

		// find its distance from cam
		scene._objQ.back().distFromCam = tre::Maths::distBetweentObjToCam(scene._objQ.back().objPos, cam.camPositionV);

		scene._transparentObjQ.push_back(std::make_pair(&scene._objQ.back(), scene._objQ.back().pObjMeshes[0]));

		scene._toSortTransparentQ = true;

	}
	else {
		scene._opaqueObjQ.push_back(std::make_pair(&scene._objQ.back(), scene._objQ.back().pObjMeshes[0]));
	}

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
			newObj.pObjMeshes = { &scene._debugMeshes[4 + selectIdx] };
			newObj.pObjMeshes[0]->material = &scene._debugMaterials[selectIdx];
			newObj.objPos = XMFLOAT3(tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20));
			newObj.objScale = XMFLOAT3(scaleVal, scaleVal, scaleVal);
			newObj.objRotation = XMFLOAT3(tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360));
			newObj._boundingVolumeColor = { tre::colorF(Colors::Green) };
			newObj.ritterBs = { newObj.pObjMeshes[0]->ritterSphere };
			newObj.naiveBs = { newObj.pObjMeshes[0]->naiveSphere };
			newObj.aabb = { newObj.pObjMeshes[0]->aabb };

			scene._objQ.push_back(newObj);

			// transparent queue -> object with texture with alpha channel or object with color.w below 1.0f
			if ((scene._objQ.back().pObjMeshes[0]->material->objTexture != nullptr && scene._objQ.back().pObjMeshes[0]->material->objTexture->hasAlphaChannel)
				|| (scene._objQ.back().pObjMeshes[0]->material->objTexture == nullptr && scene._objQ.back().pObjMeshes[0]->material->baseColor.w < 1.0f)) {

				// find its distance from cam
				scene._objQ.back().distFromCam = tre::Maths::distBetweentObjToCam(scene._objQ.back().objPos, cam.camPositionV);

				scene._transparentObjQ.push_back(std::make_pair(&scene._objQ.back(), scene._objQ.back().pObjMeshes[0]));

				scene._toSortTransparentQ = true;

			}
			else {
				scene._opaqueObjQ.push_back(std::make_pair(&scene._objQ.back(), scene._objQ.back().pObjMeshes[0]));
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
				if (scene.lightResc.pointLights.size() < scene.lightResc.maxPointLightNum) {
					if (ImGui::Button("Add Pt Light")) {
						scene.lightResc.addPointLight();

						tre::Object newLightObj;

						newLightObj.pObjMeshes = { &scene._debugMeshes[1] }; // sphere
						newLightObj.pObjMeshes[0]->material = &scene._debugMaterials[2];
						newLightObj.objPos = scene.lightResc.pointLights.back().pos;
						newLightObj.objScale = XMFLOAT3(.1f, .1f, .1f);
						newLightObj.objRotation = XMFLOAT3(.0f, .0f, .0f);
						newLightObj._boundingVolumeColor = { tre::colorF(Colors::White) };

						scene._objQ.push_back(newLightObj);
						scene._wireframeObjQ.push_back(std::make_pair(&scene._objQ.back(), scene._objQ.back().pObjMeshes[0]));
					}
					ImGui::SameLine();
					ImGui::Text("Current Light Count: %d/%d", scene.lightResc.pointLights.size(), scene.lightResc.maxPointLightNum);
				} else {
					ImGui::Text("Max Light Count: %d/%d", scene.lightResc.pointLights.size(), scene.lightResc.maxPointLightNum);
				}

				ImGui::BulletText("Dir Light");
				ImGui::SliderFloat("Yaw", &scene.dirlightYaw, .0f, 360.f);
				ImGui::SliderFloat("Pitch", &scene.dirlightPitch, .0f, 89.f);
			}

			{	// farplane intervals
				ImGui::SeparatorText("Far Planes");
				ImGui::Checkbox("CSM Debug", &csmDebugSwitch);
				ImGui::SliderFloat("Far Plane 1", &planeIntervalsF.x, planeIntervals[0], planeIntervals[2]);
				ImGui::SliderFloat("Far Plane 2", &planeIntervalsF.y, planeIntervals[1], planeIntervals[3]);
				ImGui::SliderFloat("Far Plane 3", &planeIntervalsF.z, planeIntervals[2], planeIntervals[4]);
				ImGui::SliderFloat("Far Plane 4", &planeIntervalsF.w, planeIntervals[3], 1000.0f);

				planeIntervals[1] = planeIntervalsF.x, planeIntervals[2] = planeIntervalsF.y, planeIntervals[3] = planeIntervalsF.z, planeIntervals[4] = planeIntervalsF.w;
			}

			{	// Stats
				ImGui::SeparatorText("Debug Info");
				ImGui::Text("Within Frustcum/Total: %d / %d", scene._culledOpaqueObjQ.size(), scene._opaqueObjQ.size());
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			}

			ImGui::End();
		}

		// Update Camera
		cam.camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, 1.0f, 1000000000.0f);
		cam.updateCamera();

		// update distance from camera
		if (scene._toRecalDistFromCam) {
			for (int i = 0; i < scene._transparentObjQ.size(); i++) {
				scene._transparentObjQ[i].first->distFromCam = tre::Maths::distBetweentObjToCam(scene._transparentObjQ[i].first->objPos, cam.camPositionV);
			}
			scene._toSortTransparentQ = true;
			scene._toRecalDistFromCam = false;
		}

		// sort the vector -> object with greater dist from cam is at the front of the Q
		if (scene._toSortTransparentQ) {
			std::sort(scene._transparentObjQ.begin(), scene._transparentObjQ.end(), [](const std::pair<tre::Object*, tre::Mesh*> obj1, const std::pair<tre::Object*, tre::Mesh*> obj2) { return obj1.first->distFromCam > obj2.first->distFromCam; });
			scene._toSortTransparentQ = false;
		}

		// cull objects
		scene._culledOpaqueObjQ.clear();
		scene._culledOpaqueObjQ.push_back(std::make_pair(&scene._floor, scene._floor.pObjMeshes[0]));
		for (int i = 0; i < scene._opaqueObjQ.size(); i++) {
			switch (typeOfBound) {
			case tre::AABBBoundingBox:
				tre::BoundingVolume::updateAABB(scene._opaqueObjQ[i].first->pObjMeshes[0]->aabb, scene._opaqueObjQ[i].first->aabb[0], scene._opaqueObjQ[i].first->_transformationFinal);
				break;
			case tre::RitterBoundingSphere:
				tre::BoundingVolume::updateBoundingSphere(scene._opaqueObjQ[i].first->pObjMeshes[0]->ritterSphere, scene._opaqueObjQ[i].first->ritterBs[0], scene._opaqueObjQ[i].first->_transformationFinal);
				break;
			case tre::NaiveBoundingSphere:
				tre::BoundingVolume::updateBoundingSphere(scene._opaqueObjQ[i].first->pObjMeshes[0]->naiveSphere, scene._opaqueObjQ[i].first->naiveBs[0], scene._opaqueObjQ[i].first->_transformationFinal);
				break;
			}

			if (scene._opaqueObjQ[i].first->aabb[0].isOverlapFrustum(cam.cameraFrustum)) {
				scene._opaqueObjQ[i].first->_boundingVolumeColor[0] = tre::colorF(Colors::Green);
				scene._culledOpaqueObjQ.push_back(scene._opaqueObjQ[i]);
			}
			else if (scene._opaqueObjQ[i].first->aabb[0].isInFrustum(cam.cameraFrustum)) {
				scene._opaqueObjQ[i].first->_boundingVolumeColor[0] = tre::colorF(Colors::Blue);
				scene._culledOpaqueObjQ.push_back(scene._opaqueObjQ[i]);
			}
			else {
				scene._opaqueObjQ[i].first->_boundingVolumeColor[0] = tre::colorF(Colors::Red);
			}
		}

		scene._culledTransparentObjQ.clear();
		for (int i = 0; i < scene._transparentObjQ.size(); i++) {
			if (scene._transparentObjQ[i].first->aabb[0].isOverlapFrustum(cam.cameraFrustum)) {
				scene._transparentObjQ[i].first->_boundingVolumeColor[0] = tre::colorF(Colors::Green);
				scene._culledTransparentObjQ.push_back(scene._transparentObjQ[i]);
			}
			else if (scene._transparentObjQ[i].first->aabb[0].isInFrustum(cam.cameraFrustum)) {
				scene._transparentObjQ[i].first->_boundingVolumeColor[0] = tre::colorF(Colors::Blue);
				scene._culledTransparentObjQ.push_back(scene._transparentObjQ[i]);
			}
			else {
				scene._opaqueObjQ[i].first->_boundingVolumeColor[0] = tre::colorF(Colors::Red);
			}
		}

		renderer.configureShadawSetting();

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
			tre::ConstantBuffer::setCamConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cam.camPositionV, lightViewProjs[i], lightViewProjs, planeIntervalsF, scene.dirlight, scene.lightResc.pointLights.size(), XMFLOAT2(4096, 4096), csmDebugSwitch);

			renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::SHADOW_M);
		}

		renderer.clearBufferToDraw();

		// set const buffer for camera
		tre::ConstantBuffer::setCamConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cam.camPositionV, cam.camViewProjection, lightViewProjs, planeIntervalsF, scene.dirlight, scene.lightResc.pointLights.size(), XMFLOAT2(4096, 4096), csmDebugSwitch);

		// Draw all opaque objects
		renderer.draw(scene._culledOpaqueObjQ, tre::RENDER_MODE::OPAQUE_M);

		// Draw all transparent objects
		renderer.draw(scene._culledTransparentObjQ, tre::RENDER_MODE::TRANSPARENT_M);

		if (!pauseLight) {
			// rotate point light 1
			sectorAnglePtLight[0] += 1.0f;
			if (sectorAnglePtLight[0] == 360.0f) sectorAnglePtLight[0] = .0f;
			scene.lightResc.pointLights[0].pos = tre::Maths::getRotatePosition(originPtLight[0], stackAnglePtLight[0], sectorAnglePtLight[0], 1.0f);
			scene._wireframeObjQ[0].first->objPos = scene.lightResc.pointLights[0].pos;

			// rotate point light 2
			stackAnglePtLight[1] += 1.0f;
			if (stackAnglePtLight[1] == 360.0f) stackAnglePtLight[1] = .0f;
			scene.lightResc.pointLights[1].pos = tre::Maths::getRotatePosition(originPtLight[1], stackAnglePtLight[1], sectorAnglePtLight[1], 1.0f);
			scene._wireframeObjQ[1].first->objPos = scene.lightResc.pointLights[1].pos;

			// rotate point light 3
			sectorAnglePtLight[2] += 5.0f;
			if (sectorAnglePtLight[2] == 360.0f) sectorAnglePtLight[2] = .0f;
			scene.lightResc.pointLights[2].pos = tre::Maths::getRotatePosition(originPtLight[2], stackAnglePtLight[2], sectorAnglePtLight[2], 5.0f);
			scene._wireframeObjQ[2].first->objPos = scene.lightResc.pointLights[2].pos;

			// rotate point light 4
			stackAnglePtLight[3] += 5.0f;
			if (stackAnglePtLight[3] == 360.0f) stackAnglePtLight[3] = .0f;
			scene.lightResc.pointLights[3].pos = tre::Maths::getRotatePosition(originPtLight[3], stackAnglePtLight[3], sectorAnglePtLight[3], 5.0f);
			scene._wireframeObjQ[3].first->objPos = scene.lightResc.pointLights[3].pos;
		}
		scene.lightResc.updateBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get());
		deviceAndContext.context.Get()->PSSetShaderResources(2, 1, scene.lightResc.pLightShaderRescView.GetAddressOf());

		// Draw all light object wireframe
		renderer.draw(scene._wireframeObjQ, tre::RENDER_MODE::WIREFRAME_M);

		// Draw debug
		if (showBoundingVolume) {
			renderer.debugDraw(scene._culledOpaqueObjQ, scene._debugMeshes[meshIdx], typeOfBound, tre::RENDER_MODE::WIREFRAME_M);
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
