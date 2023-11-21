#include "graphics.h"
#include "mesh.h"

namespace tre {

class WireframeRenderer {
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

	WireframeRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	Mesh* selectWireframeMesh(BoundVolumeEnum typeOfBound);

	void draw(const Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>>& renderQ);

	void drawInstanced(Graphics* graphics, const std::vector<std::pair<Object*, Mesh*>>& renderQ);

};
}