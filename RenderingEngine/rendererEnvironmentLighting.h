#pragma once

#include "graphics.h"
#include "scene.h"

namespace tre {

class RendererEnvironmentLighting {
public:
	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _deferredShaderLightingEnv;
	
	RendererEnvironmentLighting();

	void init();

	void render(Graphics& graphics, const Scene& scene, const Camera& cam);

};
}