#pragma once

#include <DirectXMath.h>

#include "texture.h"

using namespace DirectX;

namespace tre {

struct Material {
	Texture* pObjTexture;
	Texture* pObjNormalMap;
};

}