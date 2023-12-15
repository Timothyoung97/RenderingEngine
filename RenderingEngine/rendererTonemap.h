#pragma once

#include <d3d11.h>
#include "graphics.h"
#include "constbuffer.h"

using namespace DirectX;

namespace tre {

struct HDRStruct {
	float middleGrey;
	XMFLOAT3 pad;
};

class RendererTonemap {
public:
	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _hdrPixelShader;

	RendererTonemap();

	void init();

	static HDRStruct createHDRStruct(float middleGrey);

	void setConstBufferHDR(Graphics& graphics);

	void fullscreenPass(const Graphics& graphics);

	void render(Graphics& graphics);
};
}