#pragma once

#include <DirectXMath.h>

#include "graphics.h"

using namespace DirectX;

namespace tre {

struct BloomConstBufferStruct {
    XMINT2 srcViewportDimension;
    XMINT2 destViewportDimension;
    XMFLOAT2 invSrcViewportDimension;
    XMFLOAT2 invDestViewportDimension;
    float sampleRadius;
    XMFLOAT3 pad;
};

class ComputerBloom {
public:

    ComputerBloom();

	void init();

	void render(Graphics& graphics);

};
}