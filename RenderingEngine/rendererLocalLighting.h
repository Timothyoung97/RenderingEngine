#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"
#include "microprofiler.h"

namespace tre {
class RendererLocalLighting : public RendererBase {
public:

	VertexShader _vertexShader;
	PixelShader _deferredShaderLightingLocal;

	RendererLocalLighting();

	void init();

	void render(Graphics& graphics, const Scene& secne, const Camera& cam, MicroProfiler& profiler);
};
}