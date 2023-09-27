#pragma once

#include <DirectXMath.h>

#include "texture.h"

using namespace DirectX;

namespace tre {

struct Material {
	Texture* objTexture;
	Texture* objNormalMap;
	XMFLOAT4 diffuseColor;
};

}