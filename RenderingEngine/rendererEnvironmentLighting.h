#pragma once

#include "graphics.h"

namespace tre {

class RendererEnvironmentLighting {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _deferredShaderLightingEnv;
	
	RendererEnvironmentLighting(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	void render(const Graphics& graphics);

};
}