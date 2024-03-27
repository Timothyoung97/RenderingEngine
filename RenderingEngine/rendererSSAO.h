#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"
#include "microprofiler.h"
#include "utility.h"
#include "engine.h"

extern std::shared_ptr<tre::Engine> pEngine;

namespace tre {

struct SSAOStruct {
	float sampleRadius;
	XMFLOAT3 pad;
};

class RendererSSAO : public RendererBase {
public:

	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _ssaoPixelShader;
	PixelShader _textureBlurPixelShader;

	RendererSSAO();

	void init();

	static SSAOStruct createSSAOKernalStruct(float sampleRadius);

	void fullscreenPass(Graphics& graphics, const Scene& scene, const Camera& cam);
	void fullscreenBlurPass(const Graphics& graphics);

	void render(Graphics& graphics, const Scene& scene, const Camera& cam, MicroProfiler& profiler);

};
}