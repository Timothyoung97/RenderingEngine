//Using SDL
#include <SDL.h>
#include <SDL_syswm.h>

#include <dxgi.h>
#include <dxgidebug.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")

#include <iostream>
#include <fstream>

#include <stdio.h>
#include <cassert>
#include <vector>

//Custom Header
#include "timer.h"

#define CHECK_DX11_ERROR(dx11Func, ...) \
{ \
	HRESULT hresult; \
	if (!SUCCEEDED(hresult = dx11Func(__VA_ARGS__))) { \
		std::printf("Assertion failed: %d at %s:%d\n", hresult, __FILE__, __LINE__);	\
		assert(false); \
	} \
}

#define CHECK_SDL_ERR(condition) \
{ \
	if (!(condition)) { \
		std::printf("Assertion failed: %s at %s:%d\n", SDL_GetError(), __FILE__, __LINE__);	\
		assert(false); \
	} \
}

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// DXGI_DEBUG_ALL
const GUID dxgi_debug_all = { 0xe48ae283, 0xda80, 0x490b, { 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 } };

int main(int argc, char* args[])
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		CHECK_SDL_ERR(false);
	}

	//Create window
	window = SDL_CreateWindow("D3D11 with SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	CHECK_SDL_ERR(window);
	
	//Obtain Windows HWND pointer
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	//Create Device 
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;

	UINT creationFlags = D3D11_CREATE_DEVICE_DEBUG;

	D3D_FEATURE_LEVEL featureLevel;

	CHECK_DX11_ERROR(
		D3D11CreateDevice, 
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&device,
		&featureLevel,
		&context
	);

	//Create D3D11 debug layer
	ID3D11Debug* d3dDebug = nullptr;
	if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		ID3D11InfoQueue* d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);

			d3dInfoQueue->Release();
			d3dDebug->Release();
		}
	}

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

	//Create dxgiFactory
	IDXGIFactory2* dxgiFactory2 = nullptr;

	CHECK_DX11_ERROR(
		CreateDXGIFactory2,
		DXGI_CREATE_FACTORY_DEBUG,
		__uuidof(IDXGIFactory2),
		reinterpret_cast<void**>(&dxgiFactory2)
	);

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

	CHECK_DX11_ERROR(
		dxgiFactory2->CreateSwapChainForHwnd,
		device,
		hwnd,
		&swapChainDesc,
		NULL,
		NULL,
		&swapChain
	);

	swapChain3 = (IDXGISwapChain3*) swapChain;

	//Load pre-compiled shaders
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;

	CHECK_DX11_ERROR(
		D3DReadFileToBlob,
		L"./shaders/vertex_shader.bin", 
		&pVSBlob
	);

	CHECK_DX11_ERROR(
		D3DReadFileToBlob,
		L"./shaders/pixel_shader.bin",
		&pPSBlob
	);

	ID3D11VertexShader* vertex_shader_ptr = nullptr;
	ID3D11PixelShader* pixel_shader_ptr = nullptr;

	CHECK_DX11_ERROR(
		device->CreateVertexShader,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		NULL, 
		&vertex_shader_ptr
	);

	CHECK_DX11_ERROR(
		device->CreatePixelShader,
		pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		NULL,
		&pixel_shader_ptr
	);

	context->VSSetShader(vertex_shader_ptr, NULL, 0);
	context->PSSetShader(pixel_shader_ptr, NULL, 0);

	//Setting Viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	context->RSSetViewports(1, &viewport);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	//Rendering Loop
	SDL_Event e;
	bool quit = false;
	
	//Colors
	float color[4] = { .5f, .5f, .5f, 1.0f };

	// main loop
	while (!quit) 
	{
		Timer timer;

		SDL_PumpEvents();

		SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

		if (e.type == SDL_QUIT) {
			quit = true;
		}

		// Alternating buffers
		int currBackBuffer = static_cast<int>(swapChain3->GetCurrentBackBufferIndex());

		ID3D11Texture2D* currBuffer = nullptr;

		swapChain3->GetBuffer(currBackBuffer, __uuidof(ID3D11Texture2D), (LPVOID*)&currBuffer);

		// Create render target view
		ID3D11RenderTargetView* renderTargetView = nullptr;

		device->CreateRenderTargetView(currBuffer, NULL, &renderTargetView);

		context->OMSetRenderTargets(1, &renderTargetView, nullptr);
		
		context->ClearRenderTargetView(renderTargetView, color);

		context->Draw(3, 0);

		swapChain3->Present(0, 0);

		while (timer.getDeltaTime() < 1000.0 / 1) {
		}
	}

	//Cleanup
	swapChain3->Release();
	dxgiFactory2->Release();
	context->Release();
	device->Release();

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
