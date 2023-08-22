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

#include <iostream>
#include <fstream>

#include <stdio.h>
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
	float offset_x = .0f;
	float offset_y = .0f;
	float translate_speed = .001f;

	float scale_x = 1.0f;
	float scale_y = 1.0f;
	float scale_speed = .001f;

	float rotate_z = .0f;
	float rotate_speed = .1f;

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

	// Coordinate
	XMMATRIX camView;
	XMMATRIX camProjection;

	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;
	
	// Camera View
	camPosition = XMVectorSet(.0f, .0f, -3.0f, .0f);
	camTarget = XMVectorSet(.0f, .0f, .0f, .0f);
	camUp = XMVectorSet(.0f, 1.0f, .0f, .0f);

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	// Projection
	camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(SCREEN_WIDTH / SCREEN_HEIGHT), 1.0f, 1000.0f);

	// main loop
	while (!input.shouldQuit())
	{
		tre::Timer timer;

		// Update keyboard event
		input.updateInputEvent();
		
		// Model 
		if (input.getKeyState(SDL_SCANCODE_LEFT)) {
			offset_x -= translate_speed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_RIGHT)) {
			offset_x += translate_speed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_UP)) {
			offset_y += translate_speed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_DOWN)) {
			offset_y -= translate_speed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_PAGEUP)) {
			scale_x += scale_speed * deltaTime;
			scale_y += scale_speed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_PAGEDOWN)) {
			scale_x -= scale_speed * deltaTime;
			scale_y -= scale_speed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_Z)) {
			rotate_z += rotate_speed * deltaTime;
		} else if (input.getKeyState(SDL_SCANCODE_1)) {
			currTriColor = 0;
		} else if (input.getKeyState(SDL_SCANCODE_2)) {
			currTriColor = 1;
		} else if (input.getKeyState(SDL_SCANCODE_3)) {
			currTriColor = 2;
		}

		// model matrix = scale -> rotate -> translate
		XMMATRIX tf_matrix = XMMatrixMultiply(
			XMMatrixScaling(scale_x, scale_y, 1), 
			XMMatrixMultiply(
				XMMatrixRotationZ(XMConvertToRadians(rotate_z)), 
				XMMatrixTranslation(offset_x, offset_y, 0)
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
