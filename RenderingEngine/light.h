#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct Light {
	XMFLOAT3 direction;
	float pad;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};