#pragma once

#include <DirectXMath.h>

#include "graphics.h"

using namespace DirectX;

namespace tre {

struct BloomConstBufferStruct {
	int currMipLevel;
	XMINT3 pad;
};

class RendererBloom {
public:

	RendererBloom();

	void init();

	void render(Graphics& graphics);

};
}