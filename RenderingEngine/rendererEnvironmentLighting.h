#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"
#include "microprofiler.h"
#include "utility.h"
#include "engine.h"

extern std::shared_ptr<tre::Engine> pEngine;

namespace tre {

class RendererEnvironmentLighting : public RendererBase {
public:
	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _deferredShaderLightingEnv;
	
	RendererEnvironmentLighting();

	void init();

	void render(Graphics& graphics, const Scene& scene, const Camera& cam, MicroProfiler& profiler);

};
}