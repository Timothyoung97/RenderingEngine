#pragma once

#include "rendererBase.h"
#include "graphics.h"
#include "scene.h"

namespace tre {

struct SSAOKernalStruct {
	XMFLOAT4 kernalSamples[64];
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

	static SSAOKernalStruct createSSAOKernalStruct(const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius);

	void fullscreenPass(Graphics& graphics, const Scene& scene, const Camera& cam);
	void fullscreenBlurPass(const Graphics& graphics);

	void render(Graphics& graphics, const Scene& scene, const Camera& cam);

};
}