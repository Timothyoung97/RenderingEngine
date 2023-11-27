#pragma once

#include "graphics.h"
#include "scene.h"

namespace tre {

class RendererTransparency {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	VertexShader _vertexShader;
	PixelShader _forwardShader;

	RendererTransparency(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	void render(Graphics& graphics, const Scene& scene);
};
}