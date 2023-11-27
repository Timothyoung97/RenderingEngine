#pragma once

#include "graphics.h"
#include "scene.h"

namespace tre {

struct SSAOKernalStruct {
	XMFLOAT4 kernalSamples[64];
	float sampleRadius;
	XMFLOAT3 pad;
};

class RendererSSAO {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _ssaoPixelShader;
	PixelShader _textureBlurPixelShader;

	RendererSSAO(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	static SSAOKernalStruct createSSAOKernalStruct(const std::vector<XMFLOAT4>& kernalSamples, float sampleRadius);

	void fullscreenPass(Graphics& graphics, const Scene& scene, const Camera& cam);
	void fullscreenBlurPass(const Graphics& graphics);

	void render(Graphics& graphics, const Scene& scene, const Camera& cam);

};
}