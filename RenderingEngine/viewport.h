#pragma once

#include <d3d11.h>

namespace tre {
class Viewport {
public:
	D3D11_VIEWPORT defaultViewport;
	D3D11_VIEWPORT shadowViewport;

	void Init(int width, int height);
};
}