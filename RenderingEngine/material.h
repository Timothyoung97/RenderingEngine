#pragma once

#include <DirectXMath.h>

#include "texture.h"

using namespace DirectX;

namespace tre {

struct Material {
	Texture* objTexture = nullptr;
	Texture* objNormalMap = nullptr;
	XMFLOAT4 baseColor;
};

}