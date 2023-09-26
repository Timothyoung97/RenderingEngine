#pragma once

#include <vector>

#include "device.h"
#include "object.h"
#include "constbuffer.h"
#include "blendstate.h"
#include "rasterizer.h"
#include "depthbuffer.h"
#include "sampler.h"
#include "shader.h"

namespace tre {

enum RENDER_MODE {
	TRANSPARENT_M,
	OPAQUE_M,
	WIREFRAME_M,
	SHADOW_M
};

class Renderer {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	BlendState _blendstate;
	Rasterizer _rasterizer;
	DepthBuffer _depthbuffer;
	Sampler _sampler;
	tre::VertexShader _vertexShader;
	tre::PixelShader _pixelShader;
	tre::PixelShader _debugPixelShader;

	Renderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	void configureStates(RENDER_MODE renderMode);
	void draw(const std::vector<Object>& objQ, RENDER_MODE renderMode);
	void debugDraw(std::vector<Object>& objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderMode);
};
}