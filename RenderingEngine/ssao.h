#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include <vector>

#include "utility.h"
#include "dxdebug.h"
#include "window.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace tre {

class SSAO {

public:
	ID3D11Device* _device;

	ComPtr<ID3D11Texture2D> ssaoResultTexture2d;
	ComPtr<ID3D11Texture2D> ssaoBlurredTexture2d;

	void create(ID3D11Device* device);

};
}