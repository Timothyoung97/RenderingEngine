#pragma once

#include "graphics.h"
#include "scene.h"

namespace tre {
class RendererLocalLighting {
public:

	VertexShader _vertexShader;
	PixelShader _deferredShaderLightingLocal;

	RendererLocalLighting();

	void init();

	void render(Graphics& graphics, const Scene& secne, const Camera& cam);
};
}