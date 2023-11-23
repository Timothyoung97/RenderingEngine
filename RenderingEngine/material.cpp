#include "material.h"

namespace tre {

bool Material::isTransparent() {
	if ((objTexture != nullptr && objTexture->hasAlphaChannel)
		|| (objTexture == nullptr && baseColor.w < 1.0f)) {
		return true;
	}

	return false;
}
}