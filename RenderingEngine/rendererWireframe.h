#pragma once

#include <set>

#include "rendererBase.h"
#include "graphics.h"
#include "mesh.h"
#include "camera.h"
#include "scene.h"
#include "microprofiler.h"
#include "utility.h"
#include "engine.h"
#include "dxdebug.h"

extern std::shared_ptr<tre::Engine> pEngine;

namespace tre {

class RendererWireframe : public RendererBase {
public:
	// Wireframe Meshes
	Mesh wireframeSphere;
	Mesh wireframeCube;

	// Shaders
	VertexShader _vertexShader;
	PixelShader _debugPixelShader;	
	
	VertexShader _vertexShaderInstanced;
	PixelShader _debugPixelShaderInstanced;

	RendererWireframe();

	void init();

	Mesh* selectWireframeMesh(BoundVolumeEnum typeOfBound);

	void setConstBufferCamViewProj(Graphics& graphic, const Camera& cam);

	void draw(Graphics& graphics, const std::vector<Object*>& renderQ);

	void drawInstanced(Graphics& graphics, const std::vector<Object*>& renderQ, InstanceBuffer& targetInstanceBuffer);

	void render(Graphics& graphics, const Camera& cam, const Scene& scene, MicroProfiler& profiler);

};
}