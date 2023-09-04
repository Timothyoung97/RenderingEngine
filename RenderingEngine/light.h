#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct Light {
	XMFLOAT3 direction;
	float pad1;
	XMFLOAT3 pos;
	float range;
	XMFLOAT3 att;
	float pad2;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};