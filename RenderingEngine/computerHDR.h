#pragma once

#include <d3d11.h>

#include "rendererBase.h"
#include "graphics.h"
#include "constbuffer.h"

using namespace DirectX;

namespace tre {

struct LuminanceStruct {
	XMFLOAT2 log2luminance;
	float timeCoeff;
	int numPixel;
	XMINT2 viewportDimension;
	XMINT2 pad;
};

class ComputerHDR : public RendererBase {
public:
	ComputeShader _computeShaderLuminancehistogram;
	ComputeShader _computeShaderLuminanceAverage;

	ComputerHDR();

	void init();

	static LuminanceStruct createLuminanceStruct(const XMFLOAT2& luminance, float timeCoeff);

	void setConstBufferLuminSetting(Graphics& graphics);

	void dispatchHistogram(const Graphics& graphics);
	void dispatchAverage(const Graphics& graphics);

	void compute(Graphics& graphics);
};
}