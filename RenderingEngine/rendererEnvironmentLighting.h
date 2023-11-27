#pragma once

#include "graphics.h"
#include "scene.h"

namespace tre {

class RendererEnvironmentLighting {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _deferredShaderLightingEnv;
	
	RendererEnvironmentLighting(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	void render(Graphics& graphics, const Scene& scene, const Camera& cam);

};
}