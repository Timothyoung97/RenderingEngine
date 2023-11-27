#pragma once

#include "graphics.h"
#include "scene.h"

namespace tre {
class RendererLocalLighting {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	VertexShader _vertexShader;
	PixelShader _deferredShaderLightingLocal;

	RendererLocalLighting(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	void render(const Graphics& graphics, const Scene& secne, const Camera& cam);
};
}