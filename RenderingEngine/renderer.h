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
#include "viewport.h"

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
	Viewport _viewport;
	
	VertexShader _vertexShader;
	PixelShader _pixelShader;
	PixelShader _debugPixelShader;

	Renderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	void configureShadawSetting();
	void setShadowBufferDrawSection(int idx); // idx --> 0: top left, 1: top right, 2: bottom left, 3: bottom right
	void configureStates(RENDER_MODE renderMode);
	void draw(const std::vector<Object>& objQ, RENDER_MODE renderMode);
	void debugDraw(std::vector<Object>& objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderMode);
};
}