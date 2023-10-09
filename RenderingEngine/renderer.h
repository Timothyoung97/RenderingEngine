#pragma once

#include <vector>

#include "device.h"
#include "object.h"
#include "factory.h"
#include "swapchain.h"
#include "constbuffer.h"
#include "blendstate.h"
#include "rasterizer.h"
#include "depthbuffer.h"
#include "sampler.h"
#include "shader.h"
#include "viewport.h"
#include "inputlayout.h"
#include "gbuffer.h"

namespace tre {

enum RENDER_MODE {
	TRANSPARENT_M,
	OPAQUE_M,
	WIREFRAME_M,
	SHADOW_M,
	DEFERRED_OPAQUE_M,
	DEFERRED_OPAQUE_LIGHTING_ENV_M
};

class Renderer {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	Factory _factory;
	Swapchain _swapchain;
	BlendState _blendstate;
	Rasterizer _rasterizer;
	DepthBuffer _depthbuffer;
	Sampler _sampler;
	Viewport _viewport;
	
	VertexShader _vertexShader;
	InputLayout _inputLayout;
	
	VertexShader _vertexShaderFullscreenQuad;

	PixelShader _forwardShader;
	PixelShader _deferredShader;
	PixelShader _deferredShaderLighting;
	PixelShader _debugPixelShader;

	GBuffer _gBuffer;
	
	ID3D11RenderTargetView* currRenderTargetView = nullptr;

	Renderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND window);

	void reset();

	void setShadowBufferDrawSection(int idx); // idx --> 0: top left, 1: top right, 2: bottom left, 3: bottom right
	void configureStates(RENDER_MODE renderMode);

	void clearSwapChainBuffer();
	void clearShadowBuffer();

	void draw(const std::vector<std::pair<Object*, Mesh*>> objQ, RENDER_MODE renderMode);
	void deferredLightingDraw();
	void debugDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderMode);

};
}