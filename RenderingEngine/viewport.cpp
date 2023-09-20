#include "viewport.h"

namespace tre {

void Viewport::Init(int width, int height) {
	ZeroMemory(&defaultViewport, sizeof(D3D11_VIEWPORT));
	defaultViewport.TopLeftX = 0;
	defaultViewport.TopLeftY = 0;
	defaultViewport.Width = width;
	defaultViewport.Height = height;
	defaultViewport.MinDepth = 0;
	defaultViewport.MaxDepth = 1;
	
	ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
	shadowViewport.Height = 4096;
	shadowViewport.Width = 4096;
	shadowViewport.MinDepth = 0.f;
	shadowViewport.MaxDepth = 1.f;
}
}