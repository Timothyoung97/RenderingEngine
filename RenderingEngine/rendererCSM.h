#pragma once

#include "graphics.h"
#include "scene.h"
#include "camera.h"

namespace tre {

class RendererCSM{
public:
	VertexShader _vertexShaderInstanced;

	RendererCSM();
	
	void init();
	void setCSMViewport(Graphics& graphics, int idx);
	void drawInstanced(Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>>& objQ);
	void render(Graphics& graphics, Scene& scene, const Camera& cam);
};
}