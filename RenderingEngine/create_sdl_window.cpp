//Using SDL
#include <SDL.h>
#include <SDL_syswm.h>

#include <dxgi.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <iostream>
#include <stdio.h>
#include <cassert>

//#define CHECK_DX11_ERROR(condition) \
//{ \
//	if (!(condition)) { \
//		std::printf("Assertion failed: %s at %s:%d\n", result, __FILE__, __LINE__);	\
//		assert(condition); \
//	} \
//}

#define CHECK_SDL_ERR(condition) \
{ \
	if (!(condition)) { \
		std::printf("Assertion failed: %s at %s:%d\n", SDL_GetError(), __FILE__, __LINE__);	\
		assert(condition); \
	} \
}

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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

	ID3D11Debug* d3dDebug = nullptr;
	if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		ID3D11InfoQueue* d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
#endif

			D3D11_MESSAGE_ID hide[] =
			{
			D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
			// Add more message IDs here as needed
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
			d3dInfoQueue->Release();
		}
		d3dDebug->Release();
	}

	//CHECK_DX11_ERROR(SUCCEEDED(result));

	//Create dxgiFactory
	IDXGIFactory* dxgiFactory = nullptr;
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory));

	//CHECK_DX11_ERROR(SUCCEEDED(result));

	//Create SwapChain
	IDXGISwapChain* swapChain = nullptr;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH; 
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;

	result = dxgiFactory->CreateSwapChain(device, &swapChainDesc, &swapChain);

	//CHECK_DX11_ERROR(SUCCEEDED(result));

	//Rendering Loop
	SDL_Event e;
	bool quit = false;
	bool isFrontBuffer = true;


	while (!quit) {

		SDL_PumpEvents();

		SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

		if (e.type == SDL_QUIT) {
			quit = true;
		}

		// Get the back buffer from the swap chain
		ID3D11Texture2D* backBuffer;
		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

		// Create a render target view from the back buffer
		ID3D11RenderTargetView* renderTargetView;
		device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

		// Clear the back buffer (render target view) to grey color
		float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
		context->ClearRenderTargetView(renderTargetView, clearColor);

		// Present the frame
		swapChain->Present(1, 0); // Synchronize to screen refresh

		// Release resources
		renderTargetView->Release();
		backBuffer->Release();
	}

	//Cleanup
	swapChain->Release();
	dxgiFactory->Release();
	context->Release();
	device->Release();

	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();

	return 0;
}
