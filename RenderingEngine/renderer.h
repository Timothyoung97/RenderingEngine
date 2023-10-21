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
#include "ssao.h"

namespace tre {

enum RENDER_MODE {
	TRANSPARENT_M,
	OPAQUE_M,
	WIREFRAME_M,
	SHADOW_M,
	DEFERRED_OPAQUE_M,
	DEFERRED_OPAQUE_LIGHTING_ENV_M,
	DEFERRED_LIGHTING_LOCAL_M,
	SSAO_FULLSCREEN_PASS,
	SSAO_BLURRING_PASS
};

struct RendererSetting {
	bool showBoundingVolume = false;
	bool pauseLight = false;
	bool ssaoSwitch = false;
	tre::BoundVolumeEnum typeOfBound = tre::AABBBoundingBox; // default
	int meshIdx = 0; // 
	float ssaoSampleRadius = .1f, ssaoBias = .0f;
	bool csmDebugSwitch = false;
	int shadowCascadeOpaqueObjs[4] = { 0, 0, 0, 0 };
};

struct RendererStats {
	int shadowCascadeOpaqueObjs[4] = { 0, 0, 0, 0 };
	int opaqueMeshCount = 0, transparentMeshCount = 0, totalMeshCount = 0;
};

class Renderer {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;
	Factory _factory;
	Swapchain _swapchain;

	// Pipeline
	Viewport _viewport;

	InputLayout _inputLayout;
	VertexShader _vertexShader;
	VertexShader _vertexShaderFullscreenQuad;

	Rasterizer _rasterizer;

	PixelShader _forwardShader;
	PixelShader _deferredShader;
	PixelShader _deferredShaderLightingEnv;
	PixelShader _deferredShaderLightingLocal;
	PixelShader _ssaoPixelShader;
	PixelShader _textureBlurPixelShader;
	PixelShader _debugPixelShader;
	Sampler _sampler;

	BlendState _blendstate;
	DepthBuffer _depthbuffer;
	
	// Misc
	GBuffer _gBuffer;
	SSAO _ssao;
	RendererSetting setting;
	RendererStats stats;

	ID3D11RenderTargetView* currRenderTargetView = nullptr;

	Renderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND window);

	void reset();

	void setShadowBufferDrawSection(int idx); // idx --> 0: top left, 1: top right, 2: bottom left, 3: bottom right
	void configureStates(RENDER_MODE renderMode);

	void clearSwapChainBuffer();
	void clearShadowBuffer();

	void draw(const std::vector<std::pair<Object*, Mesh*>> objQ, RENDER_MODE renderMode);
	void fullscreenPass(tre::RENDER_MODE mode);
	void deferredLightingLocalDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, XMVECTOR cameraPos);
	void debugDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderMode);

};
}