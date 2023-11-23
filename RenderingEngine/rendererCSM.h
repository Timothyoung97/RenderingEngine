#pragma once

#include "graphics.h"
#include "scene.h"
#include "camera.h"

namespace tre {

class RendererCSM{
public:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	VertexShader _vertexShaderInstanced;

	RendererCSM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	
	void generateCSMViewProj(const Graphics& graphics, Scene& scene, const Camera& cam);
	void setCSMViewport(Graphics& graphics, int idx);
	void drawInstanced(Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>>& objQ);
	void render(Graphics& graphics, Scene& scene, const Camera& cam);
};
}