#define SDL_MAIN_HANDLED

#include <dxgi1_4.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "spdlog/spdlog.h"

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
#include "factory.h"
#include "swapchain.h"
#include "mesh.h"
#include "utility.h"
#include "camera.h"
#include "constbuffer.h"
#include "texture.h"
#include "object.h"
#include "shader.h"
#include "renderer.h"
#include "light.h"
#include "colors.h"
#include "boundingvolume.h"
#include "maths.h"
#include "inputlayout.h"
#include "scene.h"
#include "viewport.h"
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

	//Create dxgiFactory
	tre::Factory factory;

	//Create SwapChain
	tre::Swapchain swapchain;
	swapchain.DescSwapchain(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	swapchain.InitSwapchainViaHwnd(factory.dxgiFactory2, deviceAndContext.device, window.getWindowHandle());

	// 3D objects
	static tre::Mesh meshes[4] = {
		tre::CubeMesh(deviceAndContext.device.Get()), 
		tre::SphereMesh(deviceAndContext.device.Get(), 20, 20),
		tre::TeapotMesh(deviceAndContext.device.Get())
	};

	// Create texture
	std::string basePathStr = tre::Utility::getBasePathStr();

	tre::ModelLoader ml;

	ml.load(deviceAndContext.device.Get(), basePathStr + "glTF-models\\Duck\\Duck.gltf");

	tre::Texture textures[5] = { 
		tre::TextureLoader::createTexture(deviceAndContext.device.Get(), basePathStr + "textures\\UV_image.jpg"),
		tre::TextureLoader::createTexture(deviceAndContext.device.Get(), basePathStr + "textures\\UV_image2.jpg"),
		tre::TextureLoader::createTexture(deviceAndContext.device.Get(), basePathStr + "textures\\UV_image_a.png"),
		tre::TextureLoader::createTexture(deviceAndContext.device.Get(), basePathStr + "textures\\glTF.png"),
		tre::TextureLoader::createTexture(deviceAndContext.device.Get(), basePathStr + "textures\\wall.jpg")
	};

	tre::Texture normals[2] = {
		tre::TextureLoader::createTexture(deviceAndContext.device.Get(), basePathStr + "textures\\glTF_normal.png"),
		tre::TextureLoader::createTexture(deviceAndContext.device.Get(), basePathStr + "textures\\wall_normal.jpg")
	};

	//Create Renderer
	tre::Renderer renderer(deviceAndContext.device.Get(), deviceAndContext.context.Get());

	// Create input layout
	tre::InputLayout inputLayout(deviceAndContext.device.Get(), &renderer._vertexShader);

	//Set input layout
	deviceAndContext.context->IASetInputLayout( inputLayout.vertLayout.Get() );

	//Set topology
	deviceAndContext.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create Viewport
	tre::Viewport viewports;
	viewports.Init(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);

	//Input Handler
	tre::Input input;
	
	//Delta Time between frame
	float deltaTime = 0;

	//Create Camera
	tre::Camera cam(tre::SCREEN_WIDTH, tre::SCREEN_HEIGHT);
	


	std::vector<tre::Object> opaqueObjQ;
	std::vector<tre::Object> transparentObjQ;	
	std::vector<tre::Object> culledOpaqueObjQ;
	std::vector<tre::Object> culledTransparentObjQ;

	bool toSortTransparentQ = false;
	bool toRecalDistFromCam = false;

	// create point light
	tre::PointLight pointLight[4] = {
		{ XMFLOAT3(.0f, .0f, .0f), .0f, XMFLOAT3(.0f, .0f, .0f), 100.0f, XMFLOAT3(.0f, .2f, .0f), .0f, XMFLOAT4(.1f, .1f, .1f, .1f), XMFLOAT4(.5f, .5f, .5f, .5f) },
		{ XMFLOAT3(.0f, .0f, .0f), .0f, XMFLOAT3(.0f, .0f, .0f), 100.0f, XMFLOAT3(.0f, .2f, .0f), .0f, XMFLOAT4(.1f, .1f, .1f, .1f), XMFLOAT4(.5f, .5f, .5f, .5f) },
		{ XMFLOAT3(.0f, .0f, .0f), .0f, XMFLOAT3(.0f, .0f, .0f), 100.0f, XMFLOAT3(.0f, .2f, .0f), .0f, XMFLOAT4(.1f, .1f, .1f, .1f), XMFLOAT4(.5f, .5f, .5f, .5f) },
		{ XMFLOAT3(.0f, .0f, .0f), .0f, XMFLOAT3(.0f, .0f, .0f), 100.0f, XMFLOAT3(.0f, .2f, .0f), .0f, XMFLOAT4(.1f, .1f, .1f, .1f), XMFLOAT4(.5f, .5f, .5f, .5f) },
	};

	float stackAnglePtLight[] = { .0f, .0f, .0f, .0f };
	float sectorAnglePtLight[] = { .0f, .0f, .0f, -90.0f };
	XMFLOAT3 originPtLight[] = { XMFLOAT3(3.0f, 3.0f, 3.0f),  XMFLOAT3(-3.0f, -3.0f, -3.0f), XMFLOAT3(.0f, .0f, .0f), XMFLOAT3(-1.0f, .0f, -1.0f) };

	float stackAngleForTeapot = .0f, sectorAngleForTeapot = .0f;

	// light wireframe obj
	std::vector<tre::Object> lightObjQ;

	for (int i = 0; i < 4; i++) {
		tre::Object newLightObj;

		newLightObj.pObjMesh = &meshes[1]; // sphere
		newLightObj.objPos = originPtLight[i];
		newLightObj.objScale = XMFLOAT3(.1f, .1f, .1f);
		newLightObj.objRotation = XMFLOAT3(.0f, .0f, .0f);
		newLightObj.pObjTexture = &textures[0];
		newLightObj.pObjNormalMap = nullptr;
		newLightObj.isObjWithTexture = 0;
		newLightObj.isObjWithNormalMap = 0;
		newLightObj.objColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		lightObjQ.push_back(newLightObj);
	}

	tre::LightResource lightResc(deviceAndContext.device.Get());

	lightResc.pointLights.assign(std::begin(pointLight), std::end(pointLight));
	lightResc.updateBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get());
	deviceAndContext.context.Get()->PSSetShaderResources(2, 1, lightResc.pLightShaderRescView.GetAddressOf());

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

	// Scene
	tre::Scene scene(deviceAndContext.device.Get());
	scene.createFloor();
	scene.updateDirLight();

	float planeIntervals[5] = {1.0f, 20.f, 100.f, 250.f, 500.f};
	XMFLOAT4 planeIntervalsF = { planeIntervals[1], planeIntervals[2], planeIntervals[3], planeIntervals[4]};

	// Testing Obj
	tre::Object importModel;

	importModel.pObjMesh = &ml._meshes.begin()->second;
	importModel.objPos = XMFLOAT3(.0f, .0f, .0f);
	importModel.objScale = XMFLOAT3(.1f, .1f, .1f);
	importModel.objRotation = XMFLOAT3(.0f, .0f, .0f);
	importModel.pObjTexture = &importModel.pObjMesh->material.objTexture;
	importModel.isObjWithTexture = importModel.pObjTexture->pShaderResView.Get() != nullptr ? 1 : 0;
	importModel.pObjNormalMap = &importModel.pObjMesh->material.objNormalMap;
	importModel.isObjWithNormalMap = importModel.pObjNormalMap->pShaderResView.Get() != nullptr ? 1 : 0;
	importModel.objColor = colors[2];
	importModel.ritterBs = importModel.pObjMesh->ritterSphere;
	importModel.naiveBs = importModel.pObjMesh->naiveSphere;
	importModel.aabb = importModel.pObjMesh->aabb;

	// transparent queue -> object with texture with alpha channel or object with color.w below 1.0f
	if ((importModel.isObjWithTexture && importModel.pObjTexture->hasAlphaChannel)
		|| (!importModel.isObjWithTexture && importModel.objColor.w < 1.0f)) {

		// find its distance from cam
		importModel.distFromCam = tre::Maths::distBetweentObjToCam(importModel.objPos, cam.camPositionV);

		transparentObjQ.push_back(importModel);

		toSortTransparentQ = true;

	}
	else {
		opaqueObjQ.push_back(importModel);
	}

	// main loop
	while (!input.shouldQuit())
	{
		tre::Timer timer;

		// Update keyboard event
		input.updateInputEvent();

		// Control
		if (input.keyState[SDL_SCANCODE_W]) { // control camera movement
			cam.moveCamera(cam.directionV * deltaTime);
			toRecalDistFromCam = true;
		}
		else if (input.keyState[SDL_SCANCODE_S]) {
			cam.moveCamera(-cam.directionV * deltaTime);
			toRecalDistFromCam = true;
		}
		else if (input.keyState[SDL_SCANCODE_D]) {
			cam.moveCamera(-cam.camRightV * deltaTime);
			toRecalDistFromCam = true;
		}
		else if (input.keyState[SDL_SCANCODE_A]) {
			cam.moveCamera(cam.camRightV * deltaTime);
			toRecalDistFromCam = true;
		}
		else if (input.keyState[SDL_SCANCODE_Q]) {
			cam.moveCamera(cam.defaultUpV * deltaTime);
			toRecalDistFromCam = true;
		}
		else if (input.keyState[SDL_SCANCODE_E]) {
			cam.moveCamera(-cam.defaultUpV * deltaTime);
			toRecalDistFromCam = true;
		}
		else if (input.mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) { // control camera angle
			cam.turnCamera(input.deltaDisplacement.x, input.deltaDisplacement.y);
		}
		else if (input.keyState[SDL_SCANCODE_SPACE]) {
			// Create new obj
			tre::Object newObj;

			float scaleVal = tre::Utility::getRandomFloat(3);
			int textureIdx = tre::Utility::getRandomInt(1);

			newObj.pObjMesh = &meshes[tre::Utility::getRandomInt(1)];
			newObj.objPos = XMFLOAT3(tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20), tre::Utility::getRandomFloatRange(-20, 20));
			newObj.objScale = XMFLOAT3(scaleVal, scaleVal, scaleVal);
			newObj.objRotation = XMFLOAT3(tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360));
			newObj.pObjTexture = &textures[3 + textureIdx];
			newObj.isObjWithTexture = 1;
			newObj.pObjNormalMap = &normals[textureIdx];
			newObj.isObjWithNormalMap = 1;
			newObj.objColor = colors[2];

			newObj.ritterBs = newObj.pObjMesh->ritterSphere;
			newObj.naiveBs = newObj.pObjMesh->naiveSphere;
			newObj.aabb = newObj.pObjMesh->aabb;

			// transparent queue -> object with texture with alpha channel or object with color.w below 1.0f
			if ((newObj.isObjWithTexture && newObj.pObjTexture->hasAlphaChannel)
				|| (!newObj.isObjWithTexture && newObj.objColor.w < 1.0f)) {

				// find its distance from cam
				newObj.distFromCam = tre::Maths::distBetweentObjToCam(newObj.objPos, cam.camPositionV);

				transparentObjQ.push_back(newObj);

				toSortTransparentQ = true;

			}
			else {
				opaqueObjQ.push_back(newObj);
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		if (opaqueObjQ.size())
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Debug");

			ImGui::Checkbox("Demo Window", &show_demo_window);

			ImGui::SeparatorText("Camera");
			ImGui::SliderFloat("Camera FOV Y", &fovY, 1.0f, 179.0f);

			// Bounding Type Selection
			ImGui::SeparatorText("Bounding Volume");
			ImGui::Checkbox("Show Bounding Volume", &showBoundingVolume);
			{
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

			// light 
			{
				ImGui::SeparatorText("Lights");
				ImGui::Checkbox("Pause Light", &pauseLight);
				if (lightResc.pointLights.size() < lightResc.maxPointLightNum) {
					if (ImGui::Button("Add Pt Light")) {
						lightResc.addPointLight();

						tre::Object newLightObj;

						newLightObj.pObjMesh = &meshes[1]; // sphere
						newLightObj.objPos = lightResc.pointLights.back().pos;
						newLightObj.objScale = XMFLOAT3(.1f, .1f, .1f);
						newLightObj.objRotation = XMFLOAT3(.0f, .0f, .0f);
						newLightObj.pObjTexture = &textures[0];
						newLightObj.pObjNormalMap = nullptr;
						newLightObj.isObjWithTexture = 0;
						newLightObj.isObjWithNormalMap = 0;
						newLightObj.objColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

						lightObjQ.push_back(newLightObj);
					}
					ImGui::SameLine();
					ImGui::Text("Current Light Count: %d/%d", lightResc.pointLights.size(), lightResc.maxPointLightNum);
				} else {
					ImGui::Text("Max Light Count: %d/%d", lightResc.pointLights.size(), lightResc.maxPointLightNum);
				}

				ImGui::BulletText("Dir Light");
				ImGui::SliderFloat("Yaw", &scene.dirlightYaw, .0f, 360.f);
				ImGui::SliderFloat("Pitch", &scene.dirlightPitch, .0f, 89.f);
			}

			// farplane intervals
			{
				ImGui::SeparatorText("Far Planes");
				ImGui::Checkbox("CSM Debug", &csmDebugSwitch);
				ImGui::SliderFloat("Far Plane 1", &planeIntervalsF.x, planeIntervals[0], planeIntervals[2]);
				ImGui::SliderFloat("Far Plane 2", &planeIntervalsF.y, planeIntervals[1], planeIntervals[3]);
				ImGui::SliderFloat("Far Plane 3", &planeIntervalsF.z, planeIntervals[2], planeIntervals[4]);
				ImGui::SliderFloat("Far Plane 4", &planeIntervalsF.w, planeIntervals[3], 1000.0f);

				planeIntervals[1] = planeIntervalsF.x, planeIntervals[2] = planeIntervalsF.y, planeIntervals[3] = planeIntervalsF.z, planeIntervals[4] = planeIntervalsF.w;
			}

			ImGui::SeparatorText("Debug Info");
			ImGui::Text("Within Frustcum/Total: %d / %d", culledOpaqueObjQ.size(), opaqueObjQ.size());
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		// Alternating buffers
		int currBackBuffer = static_cast<int>(swapchain.mainSwapchain->GetCurrentBackBufferIndex());

		ID3D11Texture2D* backBuffer = nullptr;

		CHECK_DX_ERROR(swapchain.mainSwapchain->GetBuffer(
			currBackBuffer, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer
		));

		// Create render target view
		ID3D11RenderTargetView* renderTargetView = nullptr;

		CHECK_DX_ERROR(deviceAndContext.device->CreateRenderTargetView(
			backBuffer, NULL, &renderTargetView
		));

		deviceAndContext.context->ClearRenderTargetView(renderTargetView, tre::BACKGROUND_COLOR);

		{ //Shadow Config

			ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
			deviceAndContext.context->PSSetShaderResources(3, 1, nullSRV);

			deviceAndContext.context->ClearDepthStencilView(renderer._depthbuffer.pShadowDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

			deviceAndContext.context->OMSetRenderTargets(0, nullptr, renderer._depthbuffer.pShadowDepthStencilView.Get());
		}

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
			viewports.shadowViewport.TopLeftX = renderer._rasterizer.rectArr[i].left;
			viewports.shadowViewport.TopLeftY = renderer._rasterizer.rectArr[i].top;
			deviceAndContext.context->RSSetViewports(1, &viewports.shadowViewport);
			deviceAndContext.context->RSSetScissorRects(1, &renderer._rasterizer.rectArr[i]);

			// set const buffer from the light pov 
			tre::ConstantBuffer::setCamConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cam.camPositionV, lightViewProjs[i], lightViewProjs, planeIntervalsF, scene.dirlight, lightResc.pointLights.size(), XMFLOAT2(4096, 4096), csmDebugSwitch);

			renderer.draw({ scene.floor, importModel }, tre::RENDER_MODE::SHADOW_M);
		}

		deviceAndContext.context->ClearDepthStencilView(renderer._depthbuffer.pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		
		deviceAndContext.context->OMSetRenderTargets(1, &renderTargetView, renderer._depthbuffer.pDepthStencilView.Get());

		// set shadowMap
		deviceAndContext.context->PSSetShaderResources(3, 1, renderer._depthbuffer.pShadowShaderRescView.GetAddressOf());

		//Set Viewport for color draw
		deviceAndContext.context->RSSetViewports(1, &viewports.defaultViewport);

		// Set camera view const buffer
		cam.camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), static_cast<float>(tre::SCREEN_WIDTH) / tre::SCREEN_HEIGHT, 1.0f, 1000.0f);
		cam.updateCamera();

		// set const buffer for camera
		tre::ConstantBuffer::setCamConstBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cam.camPositionV, cam.camViewProjection, lightViewProjs, planeIntervalsF, scene.dirlight, lightResc.pointLights.size(), XMFLOAT2(4096, 4096), csmDebugSwitch);

		// cull objects
		culledOpaqueObjQ.clear();
		for (int i = 0; i < opaqueObjQ.size(); i++) {
			switch (typeOfBound) {
			case tre::AABBBoundingBox:
				tre::BoundingVolume::updateAABB(opaqueObjQ[i].pObjMesh->aabb, opaqueObjQ[i].aabb, opaqueObjQ[i].objScale, opaqueObjQ[i].objRotation, opaqueObjQ[i].objPos);
				break;
			case tre::RitterBoundingSphere:
				tre::BoundingVolume::updateBoundingSphere(opaqueObjQ[i].pObjMesh->ritterSphere, opaqueObjQ[i].ritterBs, opaqueObjQ[i].objScale, opaqueObjQ[i].objRotation, opaqueObjQ[i].objPos);
				break;			
			case tre::NaiveBoundingSphere:
				tre::BoundingVolume::updateBoundingSphere(opaqueObjQ[i].pObjMesh->naiveSphere, opaqueObjQ[i].naiveBs, opaqueObjQ[i].objScale, opaqueObjQ[i].objRotation, opaqueObjQ[i].objPos);
				break;
			}

			if (opaqueObjQ[i].aabb.isOverlapFrustum(cam.cameraFrustum)) {
				opaqueObjQ[i].objColor = colors[1];
				culledOpaqueObjQ.push_back(opaqueObjQ[i]);
			}
			else if (opaqueObjQ[i].aabb.isInFrustum(cam.cameraFrustum)) {
				opaqueObjQ[i].objColor = colors[2];
				culledOpaqueObjQ.push_back(opaqueObjQ[i]);
			} else {
				opaqueObjQ[i].objColor = colors[0];
			}
		}

		culledTransparentObjQ.clear();
		for (int i = 0; i < transparentObjQ.size(); i++) {
			if (transparentObjQ[i].aabb.isOverlapFrustum(cam.cameraFrustum)) {
				transparentObjQ[i].objColor = colors[1];
				culledTransparentObjQ.push_back(transparentObjQ[i]);
			}
			else if (transparentObjQ[i].aabb.isInFrustum(cam.cameraFrustum)) {
				transparentObjQ[i].objColor = colors[2];
				culledTransparentObjQ.push_back(transparentObjQ[i]);
			} else {
				opaqueObjQ[i].objColor = colors[0];
			}
		}

		// Draw all opaque objects
		renderer.draw({ scene.floor }, tre::RENDER_MODE::OPAQUE_M);

		renderer.draw(culledOpaqueObjQ, tre::RENDER_MODE::OPAQUE_M);

		if (toRecalDistFromCam) {
			for (int i = 0; i < transparentObjQ.size(); i++) {
				transparentObjQ[i].distFromCam = tre::Maths::distBetweentObjToCam(transparentObjQ[i].objPos, cam.camPositionV);
			}
			toSortTransparentQ = true;
			toRecalDistFromCam = false;
		}

		// sort the vector -> object with greater dist from cam is at the front of the Q
		if (toSortTransparentQ) {
			std::sort(transparentObjQ.begin(), transparentObjQ.end(), [](const tre::Object& obj1, const tre::Object& obj2) { return obj1.distFromCam > obj2.distFromCam; });
			toSortTransparentQ = false;
		}

		// Draw all transparent objects
		renderer.draw(culledTransparentObjQ, tre::RENDER_MODE::TRANSPARENT_M);

		if (!pauseLight) {
			// rotate point light 1
			sectorAnglePtLight[0] += 1.0f;
			if (sectorAnglePtLight[0] == 360.0f) sectorAnglePtLight[0] = .0f;
			pointLight[0].pos = tre::Maths::getRotatePosition(originPtLight[0], stackAnglePtLight[0], sectorAnglePtLight[0], 1.0f);
			lightObjQ[0].objPos = pointLight[0].pos;
			lightResc.pointLights[0].pos = lightObjQ[0].objPos;

			// rotate point light 2
			stackAnglePtLight[1] += 1.0f;
			if (stackAnglePtLight[1] == 360.0f) stackAnglePtLight[1] = .0f;
			pointLight[1].pos = tre::Maths::getRotatePosition(originPtLight[1], stackAnglePtLight[1], sectorAnglePtLight[1], 1.0f);
			lightObjQ[1].objPos = pointLight[1].pos;
			lightResc.pointLights[1].pos = lightObjQ[1].objPos;

			// rotate point light 3
			sectorAnglePtLight[2] += 5.0f;
			if (sectorAnglePtLight[2] == 360.0f) sectorAnglePtLight[2] = .0f;
			pointLight[2].pos = tre::Maths::getRotatePosition(originPtLight[2], stackAnglePtLight[2], sectorAnglePtLight[2], 5.0f);
			lightObjQ[2].objPos = pointLight[2].pos;
			lightResc.pointLights[2].pos = lightObjQ[2].objPos;

			// rotate point light 4
			stackAnglePtLight[3] += 5.0f;
			if (stackAnglePtLight[3] == 360.0f) stackAnglePtLight[3] = .0f;
			pointLight[3].pos = tre::Maths::getRotatePosition(originPtLight[3], stackAnglePtLight[3], sectorAnglePtLight[3], 5.0f);
			lightObjQ[3].objPos = pointLight[3].pos;
			lightResc.pointLights[3].pos = lightObjQ[3].objPos;
		}
		lightResc.updateBuffer(deviceAndContext.device.Get(), deviceAndContext.context.Get());
		deviceAndContext.context.Get()->PSSetShaderResources(2, 1, lightResc.pLightShaderRescView.GetAddressOf());

		// Draw all light object wireframe
		renderer.draw( lightObjQ, tre::RENDER_MODE::WIREFRAME_M);

		// Draw debug
		if (showBoundingVolume) {
			renderer.debugDraw(culledOpaqueObjQ, meshes[meshIdx], typeOfBound, tre::RENDER_MODE::WIREFRAME_M);
		}

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		CHECK_DX_ERROR(swapchain.mainSwapchain->Present( 0, 0) );

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
