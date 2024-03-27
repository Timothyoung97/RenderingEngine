#pragma once

#include <d3d11.h>

#include "rendererBase.h"
#include "graphics.h"
#include "constbuffer.h"
#include "microprofiler.h"
#include "utility.h"
#include "window.h"
#include "engine.h"

extern std::shared_ptr<tre::Engine> pEngine;

using namespace DirectX;

namespace tre {

struct TonemapStruct {
	float middleGrey;
	float bloomStrength;
	XMFLOAT2 pad;
};

class RendererTonemap : public RendererBase {
public:
	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _tonemapPixelShader;

	RendererTonemap();

	void init() override;

	static TonemapStruct createTonemapStruct(float middleGrey, float bloomStrength);

	void setConstBufferTonemap(Graphics& graphics);

	void fullscreenPass(const Graphics& graphics);

	void render(Graphics& graphics, MicroProfiler& profiler);
};
}