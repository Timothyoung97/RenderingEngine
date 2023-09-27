#pragma once

#include <DirectXMath.h>
#include <DirectXColors.h>

using namespace DirectX;

namespace tre {

static XMFLOAT4 colorF(XMVECTOR directXColor) {
	XMFLOAT4 color;
	XMStoreFloat4(&color, directXColor);
	return color;
}

}