#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"
#include "microprofiler.h"
#include "utility.h"
#include "engine.h"

extern std::shared_ptr<tre::Engine> pEngine;

namespace tre {

class RendererTransparency : public RendererBase {
public:

	VertexShader _vertexShader;
	PixelShader _forwardShader;

	RendererTransparency();

	void init();

	void render(Graphics& graphics, const Scene& scene, const Camera& cam, MicroProfiler& profiler);
};
}