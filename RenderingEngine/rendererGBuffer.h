#pragma once

#include "graphics.h"
#include "scene.h"
#include "camera.h"

namespace tre {

class RendererGBuffer {
public:

	VertexShader _vertexShaderInstanced;
	PixelShader _pixelShaderInstanced;

	RendererGBuffer();

	void init();
	void render(Graphics& graphics, Scene& scene, Camera& cam);
	
};
}