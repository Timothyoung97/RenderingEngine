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

	result = dxgiFactory2->CreateSwapChainForHwnd(device, hwnd, &swapChainDesc, NULL, NULL, &swapChain);

	CHECK_DX11_ERROR(result);

	swapChain3 = (IDXGISwapChain3*) swapChain;

	////Load pre-compiled shaders
	//std::vector<byte> vertexShaderBytes;
	//std::vector<byte> pixelShaderBytes;

	//std::ifstream verShaderFile("./shaders/vertex_shader.bin", std::ios::binary | std::ios::ate);
	//if (verShaderFile) {
	//	int length = verShaderFile.tellg();
	//	verShaderFile.seekg(0, verShaderFile.beg);
	//	//std::vector<byte> data(length);
	//	vertexShaderBytes.resize(length);

	//	verShaderFile.read(reinterpret_cast<char*>(vertexShaderBytes.data()), length);
	//	//vertexShaderBytes = data;
	//}

	ID3DBlob* pVSBlob = nullptr;
	
	/*result = D3DCreateBlob(vertexShaderBytes.size(), &pVSBlob);
	
	CHECK_DX11_ERROR(result);

	memcpy(pVSBlob->GetBufferPointer(), &vertexShaderBytes, vertexShaderBytes.size());

	printf("buffer Size: %d; vector size: %d\n", pVSBlob->GetBufferSize(), vertexShaderBytes.size());
	
	
		
	*/
	ID3DBlob* pPSBlob = nullptr;



	result = D3DReadFileToBlob(L"./shaders/vertex_shader.bin", &pVSBlob);

	CHECK_DX11_ERROR(result);

	result = D3DReadFileToBlob(L"./shaders/pixel_shader.bin", &pPSBlob);

	CHECK_DX11_ERROR(result);

	//Create shaders


	CHECK_DX11_ERROR(result);

	ID3D11PixelShader* pixel_shader_ptr = nullptr;
	ID3D11VertexShader* vertex_shader_ptr = nullptr;

	result = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertex_shader_ptr);
	result = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixel_shader_ptr);

	CHECK_DX11_ERROR(result);

	//Rendering Loop
	SDL_Event e;
	bool quit = false;
	
	//Colors
	float color1[4] = { .5f, .5f, .5f, 1.0f };
	float color2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	int colorIdx = 0;

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

		result = swapChain3->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&currBuffer);

		CHECK_DX11_ERROR(result);

		// Create render target view
		ID3D11RenderTargetView* renderTargetView = nullptr;

		result = device->CreateRenderTargetView(currBuffer, NULL, &renderTargetView);

		CHECK_DX11_ERROR(result);

		context->OMSetRenderTargets(1, &renderTargetView, nullptr);
		
		context->ClearRenderTargetView(renderTargetView, colorIdx ? color1 : color2);

		colorIdx = colorIdx ? 0 : 1;

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
