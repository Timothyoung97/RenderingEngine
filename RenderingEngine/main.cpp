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
#include "depthbuffer.h"
#include "texture.h"
#include "object.h"
#include "sampler.h"
#include "rasterizer.h"
#include "shader.h"
#include "renderer.h"
#include "light.h"
#include "blendstate.h"
#include "colors.h"
#include "boundingvolume.h"
#include "matrix.h"

//Screen dimension constants
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1020;

using namespace DirectX;
using Microsoft::WRL::ComPtr;

//Input Layout
D3D11_INPUT_ELEMENT_DESC layout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const UINT numOfInputElement = ARRAYSIZE(layout);

int main()
{
	//Create util
	tre::Utility util;
	
	//Create Window
	tre::Window window("RenderingEngine", SCREEN_WIDTH, SCREEN_HEIGHT);

	//Create Device 
	tre::Device deviceAndContext;

	//Create dxgiFactory
	tre::Factory factory;

	//Create SwapChain
	tre::Swapchain swapchain;
	swapchain.DescSwapchain(SCREEN_WIDTH, SCREEN_HEIGHT);
	swapchain.InitSwapchainViaHwnd(factory.dxgiFactory2, deviceAndContext.device, window.getWindowHandle());

	//Create Sampler
	tre::Sampler sampler(deviceAndContext.device.Get());
	deviceAndContext.context->PSSetSamplers(0, 1, sampler.pSamplerState.GetAddressOf());

	//Load pre-compiled shaders
	tre::VertexShader vertex_shader(util.basePathWstr + L"shaders\\vertex_shader.bin", deviceAndContext.device.Get());
	tre::PixelShader pixel_shader(util.basePathWstr + L"shaders\\pixel_shader.bin", deviceAndContext.device.Get());
	tre::PixelShader light_pixel_shader(util.basePathWstr + L"shaders\\light_pixel.bin", deviceAndContext.device.Get());

	deviceAndContext.context->VSSetShader(vertex_shader.pShader.Get(), NULL, 0u);

	// 3D objects
	static tre::Mesh meshes[3] = {
		tre::CubeMesh(deviceAndContext.device.Get()), 
		tre::SphereMesh(deviceAndContext.device.Get(), 20, 20),
		tre::TeapotMesh(deviceAndContext.device.Get())
	};

	// Create texture
	tre::Texture textures[5] = { 
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\UV_image.jpg"), 
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\UV_image2.jpg"),
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\UV_image_a.png"),
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\glTF.png"),
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\wall.jpg")
	};

	tre::Texture normals[2] = {
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\glTF_normal.png"),
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\wall_normal.jpg")
	};

	// Create input layout
	ComPtr<ID3D11InputLayout> vertLayout;

	CHECK_DX_ERROR(deviceAndContext.device->CreateInputLayout(
		layout, numOfInputElement, vertex_shader.pBlob.Get()->GetBufferPointer(), vertex_shader.pBlob.Get()->GetBufferSize(), vertLayout.GetAddressOf()
	));

	//Create Depth/Stencil 
	tre::DepthBuffer depthBuffer(deviceAndContext.device.Get(), SCREEN_WIDTH, SCREEN_HEIGHT);

	//Blend State
	tre::BlendState blendState(deviceAndContext.device.Get());

	//Set input layout
	deviceAndContext.context->IASetInputLayout( vertLayout.Get() );

	//Set topology
	deviceAndContext.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create Viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	//Set Viewport
	deviceAndContext.context->RSSetViewports(1, &viewport);

	//Create rasterizer buffer
	tre::Rasterizer rasterizer(deviceAndContext.device.Get());

	//Input Handler
	tre::Input input;
	
	//Background Color
	float bgColor[4] = { .5f, .5f, .5f, 1.0f };
	
	//Delta Time between frame
	float deltaTime = 0;

	//Create Camera
	tre::Camera cam(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	//Create Renderer
	tre::Renderer renderer;

	//Create const buffer manager
	tre::ConstantBuffer cb;

	std::vector<tre::Object> opaqueObjQ;
	std::vector<tre::Object> transparentObjQ;

	bool toSortTransparentQ = false;
	bool toRecalDistFromCam = false;

	// set light
	Light dirlight{
		XMFLOAT3(-.5f, .5f, -.5f), .0f, XMFLOAT4(.5f, .5f, .5f, 1.0f), XMFLOAT4(.5f, .5f, .5f, .5f)
	};

	XMVECTOR lightDir = XMLoadFloat3(&dirlight.direction);
	XMStoreFloat3(&dirlight.direction, XMVector3Normalize(lightDir));

	PointLight pointLight[4] = {
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

	// pause light
	int pauseLight = 0;

	static int typeOfBound = 0;
	float scaleIncre = 90.0f;

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

	bool show_demo_window = false;

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
			newObj.pObjMesh = &meshes[tre::Utility::getRandomInt(1)];
			newObj.objPos = XMFLOAT3(tre::Utility::getRandomFloatRange(-5, 5), tre::Utility::getRandomFloatRange(-5, 5), tre::Utility::getRandomFloatRange(-5, 5));
			newObj.objScale = XMFLOAT3(scaleVal, scaleVal, scaleVal);
			newObj.objRotation = XMFLOAT3(tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360));
			newObj.pObjTexture = &textures[tre::Utility::getRandomInt(2)];
			newObj.isObjWithNormalMap = 0;
			newObj.pObjNormalMap = nullptr;


			// With/Without texture
			if (tre::Utility::getRandomInt(1) == 1) {
				newObj.isObjWithTexture = 1;
				newObj.objColor = XMFLOAT4();
			}
			else {
				newObj.isObjWithTexture = 0;
				newObj.objColor = colors[tre::Utility::getRandomInt(9)];
			}

			// transparent queue -> object with texture with alpha channel or object with color.w below 1.0f
			if ((newObj.isObjWithTexture && newObj.pObjTexture->hasAlphaChannel)
				|| (!newObj.isObjWithTexture && newObj.objColor.w < 1.0f)) {

				// find its distance from cam
				newObj.distFromCam = tre::Matrix::distBetweentObjToCam(newObj.objPos, cam.camPositionV);

				transparentObjQ.push_back(newObj);

				toSortTransparentQ = true;

			}
			else {
				opaqueObjQ.push_back(newObj);
			}
		}
		else if (input.keyState[SDL_SCANCODE_RSHIFT]) { // create obj with normal mapping

			if (opaqueObjQ.size() > 0) {
				continue;
			}

			tre::Object newNorObj;

			int textureIdx = tre::Utility::getRandomInt(1);
			newNorObj.pObjMesh = &meshes[0];
			newNorObj.objPos = XMFLOAT3(.0f, .0f, .0f);
			newNorObj.objScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
			newNorObj.objRotation = XMFLOAT3(.0f, .0f, .0f);
			newNorObj.pObjTexture = &textures[3 + textureIdx];
			newNorObj.isObjWithTexture = 0;
			newNorObj.pObjNormalMap = &normals[textureIdx];
			newNorObj.isObjWithNormalMap = 0;
			newNorObj.objColor = colors[5];
			newNorObj.distFromCam = tre::Matrix::distBetweentObjToCam(newNorObj.objPos, cam.camPositionV);

			newNorObj.ritterBs = newNorObj.pObjMesh->ritterSphere;
			newNorObj.naiveBs = newNorObj.pObjMesh->naiveSphere;
			newNorObj.aabb = newNorObj.pObjMesh->aabb;

			opaqueObjQ.push_back(newNorObj);

			tre::Object newNorObj2;

			newNorObj2.pObjMesh = &meshes[0];
			newNorObj2.objPos = XMFLOAT3(2.0f, 2.0f, 2.0f);
			newNorObj2.objScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
			newNorObj2.objRotation = XMFLOAT3(.0f, .0f, .0f);
			newNorObj2.pObjTexture = &textures[3 + textureIdx];
			newNorObj2.isObjWithTexture = 0;
			newNorObj2.pObjNormalMap = &normals[textureIdx];
			newNorObj2.isObjWithNormalMap = 0;
			newNorObj2.objColor = colors[5];
			newNorObj2.distFromCam = tre::Matrix::distBetweentObjToCam(newNorObj2.objPos, cam.camPositionV);

			newNorObj2.ritterBs = newNorObj2.pObjMesh->ritterSphere;
			newNorObj2.naiveBs = newNorObj2.pObjMesh->naiveSphere;
			newNorObj2.aabb = newNorObj2.pObjMesh->aabb;

			opaqueObjQ.push_back(newNorObj2);

		}
		else if (input.keyState[SDL_SCANCODE_P]) {
			pauseLight ^= 1;
		}

		bool isIntersect = false;
		if (opaqueObjQ.size() == 2) {
			isIntersect = opaqueObjQ[0].aabb.testAABB(opaqueObjQ[1].aabb);
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

			ImGui::Checkbox("Is Intersecting", &isIntersect);

			ImGui::Text("Object 1");               // Display some text (you can use a format strings too)

			ImGui::SliderFloat("Obj1 XPos", &opaqueObjQ[0].objPos.x, -10.0f, 10.0f);
			ImGui::SliderFloat("Obj1 YPos", &opaqueObjQ[0].objPos.y, -10.0f, 10.0f);
			ImGui::SliderFloat("Obj1 ZPos", &opaqueObjQ[0].objPos.z, -10.0f, 10.0f);
			ImGui::SliderFloat("Obj1 Scale", &opaqueObjQ[0].objScale.x, .1f, 3.0f);
			ImGui::SliderFloat("Obj1 Scale", &opaqueObjQ[0].objScale.y, .1f, 3.0f);
			ImGui::SliderFloat("Obj1 Scale", &opaqueObjQ[0].objScale.z, .1f, 3.0f);
			ImGui::SliderFloat("Obj1 XRot", &opaqueObjQ[0].objRotation.x, .0f, 360.0f);
			ImGui::SliderFloat("Obj1 YRot", &opaqueObjQ[0].objRotation.y, .0f, 360.0f);
			ImGui::SliderFloat("Obj1 ZRot", &opaqueObjQ[0].objRotation.z, .0f, 360.0f);
		
			ImGui::Text("Object 2");               // Display some text (you can use a format strings too)
			ImGui::SliderFloat("Obj2 XPos", &opaqueObjQ[1].objPos.x, -10.0f, 10.0f);
			ImGui::SliderFloat("Obj2 YPos", &opaqueObjQ[1].objPos.y, -10.0f, 10.0f);
			ImGui::SliderFloat("Obj2 ZPos", &opaqueObjQ[1].objPos.z, -10.0f, 10.0f);
			ImGui::SliderFloat("Obj2 Scale", &opaqueObjQ[1].objScale.x, .1f, 3.0f);
			ImGui::SliderFloat("Obj2 Scale", &opaqueObjQ[1].objScale.y, .1f, 3.0f);
			ImGui::SliderFloat("Obj2 Scale", &opaqueObjQ[1].objScale.z, .1f, 3.0f);
			ImGui::SliderFloat("Obj2 XRot", &opaqueObjQ[1].objRotation.x, .0f, 360.0f);
			ImGui::SliderFloat("Obj2 YRot", &opaqueObjQ[1].objRotation.y, .0f, 360.0f);
			ImGui::SliderFloat("Obj2 ZRot", &opaqueObjQ[1].objRotation.z, .0f, 360.0f);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		ImGui::Render();

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

		deviceAndContext.context->OMSetRenderTargets(1, &renderTargetView, depthBuffer.depthStencilView.Get());
		
		deviceAndContext.context->ClearRenderTargetView(renderTargetView, bgColor);

		deviceAndContext.context->ClearDepthStencilView(depthBuffer.depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Set camera view const buffer
		cb.constBufferRescCam.viewProjection = XMMatrixMultiply(cam.camView, cam.camProjection);
		cb.constBufferRescCam.light = dirlight;
		std::copy(std::begin(pointLight), std::end(pointLight), std::begin(cb.constBufferRescCam.pointLight));

		cb.csd.pSysMem = &cb.constBufferRescCam;

		CHECK_DX_ERROR(deviceAndContext.device->CreateBuffer(
			&cb.constantBufferDescCam, &cb.csd, cb.pConstBuffer.GetAddressOf()
		));

		deviceAndContext.context->VSSetConstantBuffers(0u, 1u, cb.pConstBuffer.GetAddressOf());
		deviceAndContext.context->PSSetConstantBuffers(0u, 1u, cb.pConstBuffer.GetAddressOf());

		// Set blend state for opaque obj
		deviceAndContext.context->OMSetBlendState(blendState.opaque.Get(), NULL, 0xffffffff);

		// Set depth test for opaque obj
		deviceAndContext.context->OMSetDepthStencilState(depthBuffer.pDSStateWithDepthTWriteEnabled.Get(), 0);

		// Set Pixel Shader
		deviceAndContext.context->PSSetShader(pixel_shader.pShader.Get(), NULL, 0u);

		// Draw all opaque objects
		renderer.draw(deviceAndContext.device.Get(), deviceAndContext.context.Get(), rasterizer.pRasterizerStateFCCW.Get(), cb, opaqueObjQ);

		// Set blend state for transparent obj
		deviceAndContext.context->OMSetBlendState(blendState.transparency.Get(), NULL, 0xffffffff);

		if (toRecalDistFromCam) {
			for (int i = 0; i < transparentObjQ.size(); i++) {
				transparentObjQ[i].distFromCam = tre::Matrix::distBetweentObjToCam(transparentObjQ[i].objPos, cam.camPositionV);
			}
			toSortTransparentQ = true;
			toRecalDistFromCam = false;
		}

		// sort the vector -> object with greater dist from cam is at the front of the Q
		if (toSortTransparentQ) {
			std::sort(transparentObjQ.begin(), transparentObjQ.end(), [](const tre::Object& obj1, const tre::Object& obj2) { return obj1.distFromCam > obj2.distFromCam; });
			toSortTransparentQ = false;
		}

		// Set depth test for transparent obj
		deviceAndContext.context->OMSetDepthStencilState(depthBuffer.pDSStateWithDepthTWriteDisabled.Get(), 0);

		// Draw all transparent objects
		renderer.draw(deviceAndContext.device.Get(), deviceAndContext.context.Get(), rasterizer.pRasterizerStateFCCW.Get(), cb, transparentObjQ);

		if (!pauseLight) {
			// rotate point light 1
			sectorAnglePtLight[0] += 1.0f;
			if (sectorAnglePtLight[0] == 360.0f) sectorAnglePtLight[0] = .0f;
			pointLight[0].pos = tre::Matrix::getRotatePosition(originPtLight[0], stackAnglePtLight[0], sectorAnglePtLight[0], 1.0f);
			lightObjQ[0].objPos = pointLight[0].pos;

			// rotate point light 2
			stackAnglePtLight[1] += 1.0f;
			if (stackAnglePtLight[1] == 360.0f) stackAnglePtLight[1] = .0f;
			pointLight[1].pos = tre::Matrix::getRotatePosition(originPtLight[1], stackAnglePtLight[1], sectorAnglePtLight[1], 1.0f);
			lightObjQ[1].objPos = pointLight[1].pos;

			// rotate point light 3
			sectorAnglePtLight[2] += 5.0f;
			if (sectorAnglePtLight[2] == 360.0f) sectorAnglePtLight[2] = .0f;
			pointLight[2].pos = tre::Matrix::getRotatePosition(originPtLight[2], stackAnglePtLight[2], sectorAnglePtLight[2], 5.0f);
			lightObjQ[2].objPos = pointLight[2].pos;

			// rotate point light 4
			stackAnglePtLight[3] += 5.0f;
			if (stackAnglePtLight[3] == 360.0f) stackAnglePtLight[3] = .0f;
			pointLight[3].pos = tre::Matrix::getRotatePosition(originPtLight[3], stackAnglePtLight[3], sectorAnglePtLight[3], 5.0f);
			lightObjQ[3].objPos = pointLight[3].pos;
		}

		// Set Pixel Shader for light
		deviceAndContext.context->PSSetShader(light_pixel_shader.pShader.Get(), NULL, 0u);

		// Set depth test for light
		deviceAndContext.context->OMSetDepthStencilState(depthBuffer.pDSStateWithoutDepthT.Get(), 0);
		
		// Draw all light object wireframe
		renderer.draw(deviceAndContext.device.Get(), deviceAndContext.context.Get(), rasterizer.pRasterizerStateWireFrame.Get(), cb, lightObjQ);

		// Draw debug
		renderer.debugDraw(deviceAndContext.device.Get(), deviceAndContext.context.Get(), rasterizer.pRasterizerStateWireFrame.Get(), cb, opaqueObjQ, meshes[0], tre::AABBBoundingBox);
		
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		CHECK_DX_ERROR(swapchain.mainSwapchain->Present( 0, 0) );

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
