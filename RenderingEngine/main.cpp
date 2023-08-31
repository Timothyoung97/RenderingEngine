#define SDL_MAIN_HANDLED

#include <dxgi1_4.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
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

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

using namespace DirectX;
using Microsoft::WRL::ComPtr;

//Input Layout
D3D11_INPUT_ELEMENT_DESC layout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;

	std::wstring vsFpath = util.basePathWstr + L"shaders\\vertex_shader.bin";
	std::wstring psFpath = util.basePathWstr + L"shaders\\pixel_shader.bin";

	CHECK_DX_ERROR( D3DReadFileToBlob(
		vsFpath.c_str(), &pVSBlob
	));

	CHECK_DX_ERROR( D3DReadFileToBlob(
		psFpath.c_str(), &pPSBlob
	));

	ID3D11VertexShader* vertex_shader_ptr = nullptr;
	ID3D11PixelShader* pixel_shader_ptr = nullptr;

	CHECK_DX_ERROR(deviceAndContext.device->CreateVertexShader(
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertex_shader_ptr
	));

	CHECK_DX_ERROR(deviceAndContext.device->CreatePixelShader(
		pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixel_shader_ptr
	));

	deviceAndContext.context->VSSetShader(vertex_shader_ptr, NULL, 0u);
	deviceAndContext.context->PSSetShader(pixel_shader_ptr, NULL, 0u);

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
		layout, numOfInputElement, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), vertLayout.GetAddressOf()
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
	rtbd.SrcBlend = D3D11_BLEND_ONE;
	rtbd.DestBlend = D3D11_BLEND_ZERO;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0] = rtbd;
	
	deviceAndContext.device->CreateBlendState(&blendDesc, &transparency);

	//Create rasterizer buffer
	ComPtr<ID3D11RasterizerState> pRasterizerStateCCW;

	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;

	CHECK_DX_ERROR(deviceAndContext.device->CreateRasterizerState(
		&rasterizerDesc, &pRasterizerStateCCW
	));

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

	// temp tranformation data
	float offsetX = .0f;
	float offsetY = .0f;
	float translateSpeed = .001f;

	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float scaleSpeed = .001f;

	float localYaw = .0f; // in degree
	float localPitch = .0f; // in degree
	float localRoll = .0f; // in degree
	float rotateZ = .0f;
	float rotateSpeed = .1f;

	//Input Handler
	tre::Input input;
	
	//Colors
	float bgColor[4] = { .5f, .5f, .5f, 1.0f };
	
	XMFLOAT4 triangleColor[3] = {
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 0, 1, 1}
	};

	int currTriColor = 0;

	//Delta Time between frame
	float deltaTime = 0;

	//Create Camera
	tre::Camera cam(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	//Create const buffer manager
	tre::ConstantBuffer cb;

	std::vector<tre::Object> opaqueObjQ;
	std::vector<tre::Object> transparentObjQ;

	//set blend factor
	float blendFactor[] = { 1, 1, 1, 1 };

	// main loop
	while (!input.shouldQuit())
	{
		tre::Timer timer;

		// Update keyboard event
		input.updateInputEvent();

		// Control
		if (input.keyState[SDL_SCANCODE_LEFT]) {
			offsetX -= translateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_RIGHT]) {
			offsetX += translateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_UP]) {
			offsetY += translateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_DOWN]) {
			offsetY -= translateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_PAGEUP]) {
			scaleX += scaleSpeed * deltaTime;
			scaleY += scaleSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_PAGEDOWN]) {
			scaleX -= scaleSpeed * deltaTime;
			scaleY -= scaleSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_Z]) {
			rotateZ += rotateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_1]) {
			currTriColor = 0;
		} else if (input.keyState[SDL_SCANCODE_2]) {
			currTriColor = 1;
		} else if (input.keyState[SDL_SCANCODE_3]) {
			currTriColor = 2;
		} else if (input.keyState[SDL_SCANCODE_W]) { // control camera movement
			cam.moveCamera(cam.directionV * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_S]) {
			cam.moveCamera(-cam.directionV * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_D]) {
			cam.moveCamera(-cam.camRightV * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_A]) {
			cam.moveCamera(cam.camRightV * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_Q]) {
			cam.moveCamera(cam.defaultUpV * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_E]) {
			cam.moveCamera(-cam.defaultUpV * deltaTime);
		} else if (input.mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) { // control camera angle
			cam.turnCamera(input.deltaDisplacement.x, input.deltaDisplacement.y);
		} else if (input.keyState[SDL_SCANCODE_SPACE]) {

			// Create new obj
			tre::Object newObj;
			newObj.pObjMesh = &meshes[tre::Utility::getRandomInt(1)];
			newObj.objPos = XMFLOAT3(tre::Utility::getRandomFloatRange(-5, 5), tre::Utility::getRandomFloatRange(-5, 5), tre::Utility::getRandomFloatRange(-5, 5));
			newObj.objScale = XMFLOAT3(tre::Utility::getRandomFloat(3), tre::Utility::getRandomFloat(3), tre::Utility::getRandomFloat(3));
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
			if ((newObj.isObjWithTexture && newObj.pObjTexture->hasAlphaChannel) || (!newObj.isObjWithTexture && newObj.objColor.w < 1.0f)) {

				// find its distance from cam
				XMVECTOR objPosV{ newObj.objPos.x, newObj.objPos.y, newObj.objPos.z };

				XMVECTOR distFromCamV = XMVector3Length(objPosV - cam.camPositionV); // length of vector is replicated in all components 
				
				XMFLOAT4 distFromCamF;
				XMStoreFloat4(&distFromCamF, distFromCamV);

				newObj.distFromCam = distFromCamF.x;

				transparentObjQ.push_back(newObj);

				// sort the vector -> object with greater dist from cam is at the front of the Q
				std::sort(transparentObjQ.begin(), transparentObjQ.end(), [](const tre::Object& obj1, const tre::Object& obj2) { return obj1.distFromCam > obj2.distFromCam; });

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
		cb.constBufferRescCam.matrix = XMMatrixMultiply(cam.camView, cam.camProjection);

		cb.csd.pSysMem = &cb.constBufferRescCam;

		CHECK_DX_ERROR(deviceAndContext.device->CreateBuffer(
			&cb.constantBufferDescCam, &cb.csd, cb.pConstBuffer.GetAddressOf()
		));

		deviceAndContext.context->VSSetConstantBuffers(0u, 1u, cb.pConstBuffer.GetAddressOf());
		deviceAndContext.context->PSSetConstantBuffers(0u, 1u, cb.pConstBuffer.GetAddressOf());

		// Set blend state for opaque obj
		float blendFactor[] = { .75f, .75f, .75f, 1.0f };
		deviceAndContext.context->OMSetBlendState(0, 0, 0xffffffff);

		// Draw all opaque objects
		for (int i = 0; i < opaqueObjQ.size(); i++) {

			const tre::Object &currObj = opaqueObjQ[i];

			//Set vertex buffer
			UINT vertexStride = sizeof(Vertex);
			UINT offset = 0;
			deviceAndContext.context->IASetVertexBuffers(0, 1, currObj.pObjMesh->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

			//Set index buffer
			deviceAndContext.context->IASetIndexBuffer(currObj.pObjMesh->pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

			//set shader resc view and sampler
			deviceAndContext.context->PSSetShaderResources(0, 1, currObj.pObjTexture->pShaderResView.GetAddressOf());

			//Config const buffer
			cb.constBufferRescModel.matrix = XMMatrixMultiply(
				XMMatrixScaling(currObj.objScale.x, currObj.objScale.y, currObj.objScale.z),
				XMMatrixMultiply(
					XMMatrixRotationRollPitchYaw(XMConvertToRadians(currObj.objRotation.x), XMConvertToRadians(currObj.objRotation.x), XMConvertToRadians(currObj.objRotation.x)),
					XMMatrixTranslation(currObj.objPos.x, currObj.objPos.y, currObj.objPos.z)
				)
			);
			cb.constBufferRescModel.isWithTexture = currObj.isObjWithTexture;
			cb.constBufferRescModel.color = currObj.objColor;

			//map to data to subresouce
			cb.csd.pSysMem = &cb.constBufferRescModel;

			CHECK_DX_ERROR(deviceAndContext.device->CreateBuffer(
				&cb.constantBufferDescModel, &cb.csd, cb.pConstBuffer.GetAddressOf()
			));

			deviceAndContext.context->VSSetConstantBuffers(1u, 1u, cb.pConstBuffer.GetAddressOf());
			deviceAndContext.context->PSSetConstantBuffers(1u, 1u, cb.pConstBuffer.GetAddressOf());

			deviceAndContext.context->DrawIndexed(currObj.pObjMesh->indexSize, 0, 0);
		}

		CHECK_DX_ERROR(swapchain.mainSwapchain->Present( 0, 0) );

		while (timer.getDeltaTime() < 1000.0 / 30) {
		}

		deltaTime = timer.getDeltaTime();
	}

	//Cleanup

	return 0;
}
