#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"
#include "camera.h"
#include "microprofiler.h"
#include "utility.h"
#include "engine.h"

extern std::shared_ptr<tre::Engine> pEngine;

namespace tre {

class RendererGBuffer : public RendererBase {
public:

	VertexShader _vertexShaderInstanced;
	PixelShader _pixelShaderInstanced;

	RendererGBuffer();

	void init();
	void render(Graphics& graphics, Scene& scene, Camera& cam, MicroProfiler& profiler);
};
}