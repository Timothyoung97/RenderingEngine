#pragma once

#include "graphics.h"
#include "scene.h"

namespace tre {

class RendererTransparency {
public:

	VertexShader _vertexShader;
	PixelShader _forwardShader;

	RendererTransparency();

	void init();

	void render(Graphics& graphics, const Scene& scene, const Camera& cam);
};
}