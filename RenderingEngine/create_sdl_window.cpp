//Using SDL
#include <SDL.h>
#include <SDL_syswm.h>

#include <dxgi.h>
#include <dxgidebug.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <iostream>
#include <stdio.h>
#include <cassert>

#include "my_timer.h"

#define CHECK_DX11_ERROR(hresult) \
{ \
	if (!SUCCEEDED(hresult)) { \
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
	HRESULT result = D3D11CreateDevice(
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

	CHECK_DX11_ERROR(result);

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
	result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));

	CHECK_DX11_ERROR(result);

	//Create SwapChain
	IDXGISwapChain1* swapChain = nullptr;

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

	result = dxgiFactory2->CreateSwapChainForHwnd(device, hwnd, &swapChainDesc, NULL, NULL, &swapChain);

	CHECK_DX11_ERROR(result);

	//Rendering Loop
	SDL_Event e;
	bool quit = false;

	//Get backbuffer
	ID3D11Texture2D* backBuffer;
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	ID3D11RenderTargetView* backBufferRTV = nullptr;

	result = device->CreateRenderTargetView(backBuffer, NULL, &backBufferRTV);

	CHECK_DX11_ERROR(result);

	//Get frontBuffer
	ID3D11Texture2D* frontBuffer;
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&frontBuffer);

	ID3D11RenderTargetView* frontBufferRTV = nullptr;

	result = device->CreateRenderTargetView(frontBuffer, NULL, &frontBufferRTV);

	CHECK_DX11_ERROR(result);

	// Array of buffer RTV
	ID3D11RenderTargetView* bufferRTV[2] = { backBufferRTV, frontBufferRTV };
	int bufferRTVIdx = 0;

	// Create MyTimer
	MyTimer mytimer(1);

	// main loop
	while (!quit) 
	{
		SDL_PumpEvents();

		SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

		if (e.type == SDL_QUIT) {
			quit = true;
		}

		ID3D11RenderTargetView* renderTargetView = bufferRTV[bufferRTVIdx];

		context->OMSetRenderTargets(1, &renderTargetView, nullptr);

		float clearColor[4] = { bufferRTVIdx ? .5f, .5f, .5f, 1.0f : 0.0f, 0.0f, 0.0f, 0.0f };
		
		context->ClearRenderTargetView(renderTargetView, clearColor);

		swapChain->Present(0, 0);
		
		bufferRTVIdx = bufferRTVIdx ? 0 : 1;

		mytimer.checkForSleep();
	}

	//Cleanup
	frontBufferRTV->Release();
	backBufferRTV->Release();
	frontBuffer->Release();
	backBuffer->Release();
	swapChain->Release();
	dxgiFactory2->Release();
	context->Release();
	device->Release();

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
