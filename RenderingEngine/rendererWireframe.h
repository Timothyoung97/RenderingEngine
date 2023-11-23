#pragma once

#include "graphics.h"
#include "mesh.h"
#include "camera.h"
#include "scene.h"

namespace tre {

class RendererWireframe {
public:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	// Wireframe Meshes
	Mesh wireframeSphere;
	Mesh wireframeCube;

	// Shaders
	VertexShader _vertexShader;
	PixelShader _debugPixelShader;	
	
	VertexShader _vertexShaderInstanced;
	PixelShader _debugPixelShaderInstanced;

	RendererWireframe(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	Mesh* selectWireframeMesh(BoundVolumeEnum typeOfBound);

	void setConstBufferCamViewProj(Graphics& graphic, const Camera& cam);

	void draw(Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>>& renderQ);

	void drawInstanced(Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>>& renderQ);

	void render(Graphics& graphics, const Camera& cam, const Scene& scene);

};
}