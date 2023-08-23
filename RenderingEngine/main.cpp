#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_syswm.h>
#include <dxgi.h>
#include <dxgidebug.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>

//Custom Header
#include "timer.h"
#include "window.h"
#include "input.h"
#include "dxdebug.h"
#include "device.h"

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// DXGI_DEBUG_ALL
const GUID dxgi_debug_all = { 0xe48ae283, 0xda80, 0x490b, { 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 } };

using namespace DirectX;

struct constBufferShaderResc {
	XMMATRIX transformation;
	XMMATRIX viewProjection;
	XMFLOAT4 rgbaColor;
};

int main()
{
	//Create Window
	tre::Window window("RenderingEngine", SCREEN_WIDTH, SCREEN_HEIGHT);

	//Create Device 
	tre::Device device;

	//Create dxgiFactory
	IDXGIFactory2* dxgiFactory2 = nullptr;

	CHECK_DX_ERROR(
		CreateDXGIFactory2,
		DXGI_CREATE_FACTORY_DEBUG,
		__uuidof(IDXGIFactory2),
		reinterpret_cast<void**>(&dxgiFactory2)
	);

	//Create DXGI debug layer
	IDXGIDebug1* dxgiDebug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
	{
		dxgiDebug->EnableLeakTrackingForThread();
		IDXGIInfoQueue* dxgiInfoQueue = nullptr;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{

			dxgiInfoQueue->SetBreakOnSeverity(dxgi_debug_all, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
			dxgiInfoQueue->SetBreakOnSeverity(dxgi_debug_all, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(dxgi_debug_all, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true);

			dxgiInfoQueue->Release();
			dxgiDebug->Release();
		}
	}

	//Create SwapChain
	IDXGISwapChain1* swapChain = nullptr;
	IDXGISwapChain3* swapChain3 = nullptr;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = SCREEN_WIDTH;
	swapChainDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	CHECK_DX_ERROR(
		dxgiFactory2->CreateSwapChainForHwnd,
		device.getDevice(),
		window.getWindowHandle(),
		&swapChainDesc,
		NULL,
		NULL,
		&swapChain
	);

	swapChain3 = (IDXGISwapChain3*) swapChain;

	//Load pre-compiled shaders
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;

	CHECK_DX_ERROR(
		D3DReadFileToBlob,
		L"../RenderingEngine/shaders/vertex_shader.bin", 
		&pVSBlob
	);

	CHECK_DX_ERROR(
		D3DReadFileToBlob,
		L"../RenderingEngine/shaders/pixel_shader.bin",
		&pPSBlob
	);

	ID3D11VertexShader* vertex_shader_ptr = nullptr;
	ID3D11PixelShader* pixel_shader_ptr = nullptr;

	CHECK_DX_ERROR(
		device.getDevice()->CreateVertexShader,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		NULL, 
		&vertex_shader_ptr
	);

	CHECK_DX_ERROR(
		device.getDevice()->CreatePixelShader,
		pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		NULL,
		&pixel_shader_ptr
	);

	device.getContext()->VSSetShader(vertex_shader_ptr, NULL, 0u);
	device.getContext()->PSSetShader(pixel_shader_ptr, NULL, 0u);

	//Setting Viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	device.getContext()->RSSetViewports(1, &viewport);
	device.getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// temp tranformation data
	float offsetX = .0f;
	float offsetY = .0f;
	float translateSpeed = .001f;

	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float scaleSpeed = .001f;

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
	double deltaTime = 0;

	// Camera's properties
	XMVECTOR defaultUpV = XMVectorSet(.0f, 1.0f, .0f, .0f);
	
	XMVECTOR camPositionV = XMVectorSet(.0f, .0f, -3.0f, .0f);
	XMVECTOR camUpV = XMVectorSet(.0f, 1.0f, .0f, .0f);
	XMVECTOR camRightV = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	float yaw = 90;
	float pitch = 0;

	XMFLOAT3 directionF;
	directionF.x = XMScalarCos(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));
	directionF.y = XMScalarSin(XMConvertToRadians(pitch));
	directionF.z = XMScalarSin(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));

	XMVECTOR directionV;
	directionV = XMVector3Normalize(XMLoadFloat3(&directionF));

	float cameraMoveSpeed = .001f;

	// Camera View Matrix
	XMMATRIX camView;

	// Projection Matrix
	XMMATRIX camProjection;
	camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(SCREEN_WIDTH / SCREEN_HEIGHT), 1.0f, 1000.0f);

	// main loop
	while (!input.shouldQuit())
	{
		tre::Timer timer;

		// Update keyboard event
		input.updateInputEvent();

		XMFLOAT4 camPositionF;
		XMStoreFloat4(&camPositionF, camPositionV);

		// Control
		if (input.getKeyState(SDL_SCANCODE_LEFT)) {
			offsetX -= translateSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_RIGHT)) {
			offsetX += translateSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_UP)) {
			offsetY += translateSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_DOWN)) {
			offsetY -= translateSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_PAGEUP)) {
			scaleX += scaleSpeed * deltaTime;
			scaleY += scaleSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_PAGEDOWN)) {
			scaleX -= scaleSpeed * deltaTime;
			scaleY -= scaleSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_Z)) {
			rotateZ += rotateSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_1)) {
			currTriColor = 0;
		} else if (input.getKeyState(SDL_SCANCODE_2)) {
			currTriColor = 1;
		} else if (input.getKeyState(SDL_SCANCODE_3)) {
			currTriColor = 2;
		} else if (input.getKeyState(SDL_SCANCODE_W)) {
			camPositionF.z += cameraMoveSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_S)) {
			camPositionF.z -= cameraMoveSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_D)) {
			camPositionF.x -= cameraMoveSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_A)) {
			camPositionF.x += cameraMoveSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_Q)) {
			camPositionF.y -= cameraMoveSpeed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_E)) {
			camPositionF.y += cameraMoveSpeed * deltaTime;
		} else if (input.getMouseButtonState(SDL_BUTTON_RIGHT)) {
			std::pair<Sint32, Sint32> relMotion = input.getRelMouseMotion();

			yaw += relMotion.first * .01f;
			pitch += relMotion.second * .01f;

			directionF.x = XMScalarCos(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));
			directionF.y = XMScalarSin(XMConvertToRadians(pitch));
			directionF.z = XMScalarSin(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));

			directionV = XMVector3Normalize(XMLoadFloat3(&directionF));
		}

		// Update camera
		camRightV = XMVector3Normalize(XMVector3Cross(directionV, defaultUpV));
		camUpV = XMVector3Normalize(XMVector3Cross(camRightV, directionV));

		camPositionV = XMLoadFloat4(&camPositionF);

		camView = XMMatrixLookAtLH(camPositionV, camPositionV + directionV, camUpV);

		// model matrix = scale -> rotate -> translate
		XMMATRIX tf_matrix = XMMatrixMultiply(
			XMMatrixScaling(scaleX, scaleY, 1),
			XMMatrixMultiply(
				XMMatrixRotationZ(XMConvertToRadians(rotateZ)),
				XMMatrixTranslation(offsetX, offsetY, 0)
			)
		);

		// Constant Buffer Shader Resource
		constBufferShaderResc cbsr;
		cbsr.transformation = tf_matrix;
		cbsr.viewProjection = XMMatrixMultiply(camView, camProjection);
		cbsr.rgbaColor = triangleColor[currTriColor];

		// Constant buffer
		D3D11_BUFFER_DESC constantBufferDesc;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantBufferDesc.MiscFlags = 0u;
		constantBufferDesc.ByteWidth = sizeof(cbsr);
		constantBufferDesc.StructureByteStride = 0u;

		ID3D11Buffer* pConstBuffer;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &cbsr;
		CHECK_DX_ERROR(
			device.getDevice()->CreateBuffer,
			&constantBufferDesc,
			&csd,
			&pConstBuffer
		);

		device.getContext()->VSSetConstantBuffers(0u, 1u, &pConstBuffer);
		device.getContext()->PSSetConstantBuffers(0u, 1u, &pConstBuffer);

		// Alternating buffers
		int currBackBuffer = static_cast<int>(swapChain3->GetCurrentBackBufferIndex());

		ID3D11Texture2D* currBuffer = nullptr;

		CHECK_DX_ERROR(
			swapChain3->GetBuffer,
			currBackBuffer,
			__uuidof(ID3D11Texture2D),
			(LPVOID*)&currBuffer
		)

		// Create render target view
		ID3D11RenderTargetView* renderTargetView = nullptr;

		CHECK_DX_ERROR(
			device.getDevice()->CreateRenderTargetView,
			currBuffer,
			NULL,
			&renderTargetView
		);

		device.getContext()->OMSetRenderTargets(1, &renderTargetView, nullptr);
		
		device.getContext()->ClearRenderTargetView(renderTargetView, bgColor);

		device.getContext()->Draw(3, 0);

		CHECK_DX_ERROR(
			swapChain3->Present,
			0,
			0
		);

		while (timer.getDeltaTime() < 1000.0 / 30) {
		}

		deltaTime = timer.getDeltaTime();

		pConstBuffer->Release();
	}

	//Cleanup
	swapChain3->Release();
	dxgiFactory2->Release();

	return 0;
}
