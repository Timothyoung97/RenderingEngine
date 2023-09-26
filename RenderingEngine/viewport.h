#pragma once

#include <d3d11.h>

namespace tre {
class Viewport {
public:
	D3D11_VIEWPORT defaultViewport;
	D3D11_VIEWPORT shadowViewport;

	void create(int width, int height);
};
}