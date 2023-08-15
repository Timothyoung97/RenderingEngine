//Using SDL
#include <SDL.h>
#include <SDL_syswm.h>

#include <dxgi.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <iostream>
#include <stdio.h>
#include <cassert>

#define CHECK_ERROR(condition, message) \
	do { \
		if (!(condition)) { \
			std::cerr << "Error: " << message << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
			assert(condition); \
		} \
	} while (false)

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

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT result = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&device,
		&featureLevel,
		&context
	);

	CHECK_ERROR(!FAILED(result), "Failed to create device");

	//Create dxgiFactory
	IDXGIFactory* dxgiFactory = nullptr;
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory));

	CHECK_ERROR(!FAILED(result), "Failed to create dxgiFactory");

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

	CHECK_ERROR(!FAILED(result), "Failed to create swap chain");

	//Rendering Loop
	SDL_Event e;
	bool quit = false;

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
