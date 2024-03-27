#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"
#include "camera.h"
#include "microprofiler.h"
#include "window.h"
#include "utility.h"
#include "engine.h"

extern std::shared_ptr<tre::Engine> pEngine;

namespace tre {

class RendererCSM : public RendererBase {
public:
	VertexShader _vertexShaderInstanced;

	RendererCSM();
	
	void init();
	void setCSMViewport(Graphics& graphics, int idx);
	void drawInstanced(Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>>& objQ, int csmIdx);
	void render(Graphics& graphics, Scene& scene, const Camera& cam, MicroProfiler& profiler);
};
}