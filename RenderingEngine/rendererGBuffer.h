#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"
#include "camera.h"
#include "microprofiler.h"

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