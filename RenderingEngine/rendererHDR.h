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

struct LuminanceStruct {
	XMFLOAT2 luminance;
	float timeCoeff;
	int numPixel;
	XMINT2 viewportDimension;
	XMINT2 pad;
};

class RendererHDR {
public:
	ComputeShader _computeShaderLuminancehistogram;
	ComputeShader _computeShaderLuminanceAverage;

	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _hdrPixelShader;

	RendererHDR();

	void init();

	static HDRStruct createHDRStruct(float middleGrey);
	static LuminanceStruct createLuminanceStruct(const XMFLOAT2& luminance, float timeCoeff);

	void setConstBufferLuminSetting(Graphics& graphics);
	void setConstBufferHDR(Graphics& graphics);

	void dispatchHistogram(const Graphics& graphics);
	void dispatchAverage(const Graphics& graphics);
	void fullscreenPass(const Graphics& graphics);

	void render(Graphics& graphics);
};
}