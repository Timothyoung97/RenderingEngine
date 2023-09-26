#include "viewport.h"

namespace tre {

void Viewport::create(int width, int height) {
	ZeroMemory(&defaultViewport, sizeof(D3D11_VIEWPORT));
	defaultViewport.TopLeftX = 0;
	defaultViewport.TopLeftY = 0;
	defaultViewport.Width = width;
	defaultViewport.Height = height;
	defaultViewport.MinDepth = 0;
	defaultViewport.MaxDepth = 1;
	
	ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
	shadowViewport.TopLeftX = 0;
	shadowViewport.TopLeftY = 0;
	shadowViewport.Height = 2048;
	shadowViewport.Width = 2048;
	shadowViewport.MinDepth = 0.f;
	shadowViewport.MaxDepth = 1.f;
}
}