#define SDL_MAIN_HANDLED

#include <dxgi1_4.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "spdlog/spdlog.h"

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
#include "sampler.h"
#include "rasterizer.h"
#include "shader.h"
#include "renderer.h"
#include "light.h"

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

using namespace DirectX;
using Microsoft::WRL::ComPtr;

//Input Layout
D3D11_INPUT_ELEMENT_DESC layout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const UINT numOfInputElement = ARRAYSIZE(layout);

XMFLOAT4 colors[10] = {
{0, 0, 0, 0},
{1, 0, 0, .1f},
{1, 1, 0, .1f},
{1, 1, 1, 1},
{1, 0, 1, .1f},
{0, 1, 0, 1},
{0, 0, 1, .1f},
{.5, 0, 0, .1f},
{0, .5, 0, .1f},
{0, 0, .5, 1}
};

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
	tre::Shader shader(util.basePathWstr + L"shaders\\vertex_shader.bin", util.basePathWstr + L"shaders\\pixel_shader.bin", deviceAndContext.device.Get());
	deviceAndContext.context->VSSetShader(shader.pVS.Get(), NULL, 0u);
	deviceAndContext.context->PSSetShader(shader.pPS.Get(), NULL, 0u);

	// 3D objects
	tre::Mesh meshes[2] = {
		tre::CubeMesh(deviceAndContext.device.Get()), 
		tre::SphereMesh(deviceAndContext.device.Get(), 10, 10) 
	};

	// Create texture
	tre::Texture textures[3] = { 
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\UV_image.jpg"), 
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\UV_image2.jpg"),
		tre::Texture(deviceAndContext.device.Get(), util.basePathStr + "textures\\UV_image_a.png")
	};

	// Create input layout
	ComPtr<ID3D11InputLayout> vertLayout;

	CHECK_DX_ERROR(deviceAndContext.device->CreateInputLayout(
		layout, numOfInputElement, shader.pVSBlob.Get()->GetBufferPointer(), shader.pVSBlob.Get()->GetBufferSize(), vertLayout.GetAddressOf()
	));

	//Create Depth/Stencil 
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	ComPtr<ID3D11Texture2D> depthStencilBuffer;

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = SCREEN_WIDTH;
	depthStencilDesc.Height = SCREEN_HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	CHECK_DX_ERROR(deviceAndContext.device->CreateTexture2D(
		&depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf()
	));

	CHECK_DX_ERROR(deviceAndContext.device->CreateDepthStencilView(
		depthStencilBuffer.Get(), nullptr, depthStencilView.GetAddressOf()
	));

	//Blend State
	ID3D11BlendState* transparency;

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	rtbd.BlendEnable = TRUE;
	rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0] = rtbd;
	
	deviceAndContext.device->CreateBlendState(&blendDesc, &transparency);

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

	//Create & Set rasterizer buffer
	tre::Rasterizer rasterizer(deviceAndContext.device.Get());
	deviceAndContext.context->RSSetState(rasterizer.pRasterizerStateFCCW.Get());

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

	bool toSortTransparentQ = FALSE;
	bool toRecalDistFromCam = FALSE;

	//set blend factor
	float blendFactor[] = { 1, 1, 1, 1 };

	// set light
	Light light{
		XMFLOAT3(.0f, .0f, .0f), .0f, XMFLOAT4(.5f, .5f, .5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
	};

	float stackAngle = 45.0f;
	float sectorAngle = .0f;

	light.direction.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	light.direction.y = XMScalarSin(XMConvertToRadians(stackAngle));
	light.direction.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));

	// main loop
	while (!input.shouldQuit())
	{
		tre::Timer timer;

		// Update keyboard event
		input.updateInputEvent();

		// Control
		if (input.keyState[SDL_SCANCODE_W]) { // control camera movement
			cam.moveCamera(cam.directionV * deltaTime);
			toRecalDistFromCam = TRUE;
		} else if (input.keyState[SDL_SCANCODE_S]) {
			cam.moveCamera(-cam.directionV * deltaTime);
			toRecalDistFromCam = TRUE;
		} else if (input.keyState[SDL_SCANCODE_D]) {
			cam.moveCamera(-cam.camRightV * deltaTime);
			toRecalDistFromCam = TRUE;
		} else if (input.keyState[SDL_SCANCODE_A]) {
			cam.moveCamera(cam.camRightV * deltaTime);
			toRecalDistFromCam = TRUE;
		} else if (input.keyState[SDL_SCANCODE_Q]) {
			cam.moveCamera(cam.defaultUpV * deltaTime);
			toRecalDistFromCam = TRUE;
		} else if (input.keyState[SDL_SCANCODE_E]) {
			cam.moveCamera(-cam.defaultUpV * deltaTime);
			toRecalDistFromCam = TRUE;
		} else if (input.mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) { // control camera angle
			cam.turnCamera(input.deltaDisplacement.x, input.deltaDisplacement.y);
		} else if (input.keyState[SDL_SCANCODE_SPACE]) {
			// Create new obj
			tre::Object newObj;

			float scaleVal = tre::Utility::getRandomFloat(3);
			newObj.pObjMesh = &meshes[tre::Utility::getRandomInt(1)];
			newObj.objPos = XMFLOAT3(tre::Utility::getRandomFloatRange(-5, 5), tre::Utility::getRandomFloatRange(-5, 5), tre::Utility::getRandomFloatRange(-5, 5));
			newObj.objScale = XMFLOAT3(scaleVal, scaleVal, scaleVal);
			newObj.objRotation = XMFLOAT3(tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360), tre::Utility::getRandomFloat(360));

			// With/Without texture
			if (tre::Utility::getRandomInt(1) == 1) {
				newObj.pObjTexture = &textures[tre::Utility::getRandomInt(2)];
				newObj.isObjWithTexture = 1;
				newObj.objColor = XMFLOAT4();
			} else {
				newObj.pObjTexture = &textures[tre::Utility::getRandomInt(2)];
				newObj.isObjWithTexture = 0;
				newObj.objColor = colors[tre::Utility::getRandomInt(9)];
			}
			
			// transparent queue -> object with texture with alpha channel or object with color.w below 1.0f
			if ((newObj.isObjWithTexture && newObj.pObjTexture->hasAlphaChannel) 
				|| (!newObj.isObjWithTexture && newObj.objColor.w < 1.0f)) {

				// find its distance from cam
				newObj.distFromCam = tre::Utility::distBetweentObjToCam(newObj.objPos, cam.camPositionV);

				transparentObjQ.push_back(newObj);

				toSortTransparentQ = TRUE;

			} else {
				opaqueObjQ.push_back(newObj);
			}
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

		deviceAndContext.context->OMSetRenderTargets(1, &renderTargetView, depthStencilView.Get());
		
		deviceAndContext.context->ClearRenderTargetView(renderTargetView, bgColor);

		deviceAndContext.context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Set camera view const buffer
		cb.constBufferRescCam.viewProjection = XMMatrixMultiply(cam.camView, cam.camProjection);
		cb.constBufferRescCam.light = light;

		cb.csd.pSysMem = &cb.constBufferRescCam;

		CHECK_DX_ERROR(deviceAndContext.device->CreateBuffer(
			&cb.constantBufferDescCam, &cb.csd, cb.pConstBuffer.GetAddressOf()
		));

		deviceAndContext.context->VSSetConstantBuffers(0u, 1u, cb.pConstBuffer.GetAddressOf());
		deviceAndContext.context->PSSetConstantBuffers(0u, 1u, cb.pConstBuffer.GetAddressOf());

		// Set blend state for opaque obj
		deviceAndContext.context->OMSetBlendState(0, NULL, 0xffffffff);

		// Draw all opaque objects
		renderer.draw(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cb, opaqueObjQ);

		// Set blend state for transparent obj
		deviceAndContext.context->OMSetBlendState(transparency, NULL, 0xffffffff);

		if (toRecalDistFromCam) {
			for (int i = 0; i < transparentObjQ.size(); i++) {
				transparentObjQ[i].distFromCam = tre::Utility::distBetweentObjToCam(transparentObjQ[i].objPos, cam.camPositionV);
			}
			toSortTransparentQ = TRUE;
			toRecalDistFromCam = FALSE;
		}

		// sort the vector -> object with greater dist from cam is at the front of the Q
		if (toSortTransparentQ) {
			std::sort(transparentObjQ.begin(), transparentObjQ.end(), [](const tre::Object& obj1, const tre::Object& obj2) { return obj1.distFromCam > obj2.distFromCam; });
			toSortTransparentQ = FALSE;
		}

		// Draw all transparent objects
		renderer.draw(deviceAndContext.device.Get(), deviceAndContext.context.Get(), cb, transparentObjQ);

		CHECK_DX_ERROR(swapchain.mainSwapchain->Present( 0, 0) );

		while (timer.getDeltaTime() < 1000.0 / 30) {
		}

		deltaTime = timer.getDeltaTime();

		sectorAngle += 10.0f;
		if (sectorAngle == 360.0f) sectorAngle = 0;

		light.direction.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
		light.direction.y = XMScalarSin(XMConvertToRadians(stackAngle));
		light.direction.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
		
	}

	//Cleanup

	return 0;
}
