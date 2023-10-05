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

enum RENDER_OBJ_TYPE {
	TRANSPARENT_T,
	OPAQUE_T,
	WIREFRAME_T,
	SHADOW_T
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
	
	InputLayout _inputLayout;
	VertexShader _vertexShader;
	PixelShader _pixelShader;
	PixelShader _pixelDeferredAlbedoShader;
	PixelShader _pixelDeferredNormalShader;
	PixelShader _debugPixelShader;

	GBuffer _gBuffer;
	
	Renderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND window);

	void configureShadawSetting();
	void setShadowBufferDrawSection(int idx); // idx --> 0: top left, 1: top right, 2: bottom left, 3: bottom right
	void clearBufferToDraw();
	void configureStates(RENDER_OBJ_TYPE renderMode);
	void draw(const std::vector<std::pair<Object*, Mesh*>> objQ, RENDER_OBJ_TYPE renderMode);
	void debugDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_OBJ_TYPE renderMode);

	// deferred draw
	void configureDeferredDraw(tre::GBUFFER_TYPE textureType);
};
}