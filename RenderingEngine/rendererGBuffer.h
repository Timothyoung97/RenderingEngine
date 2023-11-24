#pragma once

#include "graphics.h"
#include "scene.h"
#include "camera.h"

namespace tre {

class RendererGBuffer {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	VertexShader _vertexShaderInstanced;
	PixelShader _pixelShaderInstanced;

	RendererGBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	void render(Graphics& graphics, Scene& scene, Camera& cam);
	
};
}