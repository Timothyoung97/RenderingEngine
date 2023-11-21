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
#include "hdr.h"
#include "instanceBuffer.h"

namespace tre {

enum RENDER_MODE {
	TRANSPARENT_M,
	OPAQUE_M,
	WIREFRAME_M,
	SHADOW_M,
	INSTANCED_SHADOW_M,
	DEFERRED_OPAQUE_M,
	INSTANCED_DEFERRED_OPAQUE_M,
	DEFERRED_OPAQUE_LIGHTING_ENV_M,
	DEFERRED_LIGHTING_LOCAL_M,
	SSAO_FULLSCREEN_PASS,
	SSAO_BLURRING_PASS,
	TONE_MAPPING_PASS
};

inline const char* ToString(RENDER_MODE rm)
{
	switch (rm)
	{
	case TRANSPARENT_M:						return "Transparent";
	case OPAQUE_M:							return "Opaque";
	case WIREFRAME_M:						return "Wireframe";
	case SHADOW_M:							return "Shadow";
	case INSTANCED_SHADOW_M:				return "Instanced Shadow";
	case DEFERRED_OPAQUE_M:					return "G-Buffer";
	case INSTANCED_DEFERRED_OPAQUE_M:		return "Instanced G-Buffer";
	case DEFERRED_OPAQUE_LIGHTING_ENV_M:	return "Environment Lighting";
	case DEFERRED_LIGHTING_LOCAL_M:			return "Local Lighting";
	case SSAO_FULLSCREEN_PASS:				return "SSAO";
	case SSAO_BLURRING_PASS:				return "SSAO Blur";
	case TONE_MAPPING_PASS:					return "Tone mapping";
	default:								return "[Unknown Rendering Mode]";
	}
}

struct GraphicsSetting {
	bool showBoundingVolume = false;
	bool pauseLight = false;
	bool ssaoSwitch = false;
	tre::BoundVolumeEnum typeOfBound = tre::AABBBoundingBox; // default
	int meshIdx = 0; // 
	float ssaoSampleRadius = .1f;
	bool csmDebugSwitch = false;
	float csmPlaneIntervals[5] = { 1.0f, 20.f, 100.f, 250.f, 500.f };
	float middleGrey = .05;
	float luminaceMin = 1.f / 256.f, luminanceMax = powf(2.f, 3.5f);
	float timeCoeff = 0.05f;
	XMFLOAT4 csmPlaneIntervalsF = { csmPlaneIntervals[1], csmPlaneIntervals[2], csmPlaneIntervals[3], csmPlaneIntervals[4] };
};

struct GraphicsStats {
	int shadowCascadeOpaqueObjs[4] = { 0, 0, 0, 0 };
	int opaqueMeshCount = 0, transparentMeshCount = 0, totalMeshCount = 0;
};

class Graphics {
public:

	ID3D11Device* _device;
	ID3D11DeviceContext* _context;
	Factory _factory;
	Swapchain _swapchain;

	// Pipeline
	Viewport _viewport;

	InputLayout _inputLayout;
	VertexShader _vertexShader;
	VertexShader _shadowCastShader;
	VertexShader _vertexShaderFullscreenQuad;
	VertexShader _vertexShaderInstanced;

	Rasterizer _rasterizer;

	PixelShader _forwardShader;
	PixelShader _deferredShader;
	PixelShader _deferredShaderLightingEnv;
	PixelShader _deferredShaderLightingLocal;
	PixelShader _ssaoPixelShader;
	PixelShader _textureBlurPixelShader;
	PixelShader _hdrPixelShader;
	PixelShader _debugPixelShader;
	PixelShader _instancedPixelShader;

	Sampler _sampler;

	BlendState _blendstate;
	DepthBuffer _depthbuffer;
	
	// Buffers
	GBuffer _gBuffer;
	SSAO _ssao;
	HdrBuffer _hdrBuffer;
	InstanceBuffer _instanceBuffer;

	// Misc
	GraphicsSetting setting;
	GraphicsStats stats;

	ID3D11RenderTargetView* currRenderTargetView = nullptr;

	Graphics(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND window);

	void reset();

	void setShadowBufferDrawSection(int idx); // idx --> 0: top left, 1: top right, 2: bottom left, 3: bottom right
	void configureStates(RENDER_MODE renderMode);

	void clearSwapChainBuffer();
	void clearShadowBuffer();

	void draw(const std::vector<std::pair<Object*, Mesh*>> objQ, RENDER_MODE renderMode);
	void fullscreenPass(tre::RENDER_MODE mode);
	void deferredLightingLocalDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, XMVECTOR cameraPos);
	void instancedDraw(const std::vector<std::pair<Object*, Mesh*>>& objQ, RENDER_MODE renderMode);

	//////////// Deprecated ////////////
	
	//void debugDraw(const std::vector<std::pair<Object*, Mesh*>> objQ, Mesh& mesh, BoundVolumeEnum typeOfBound, RENDER_MODE renderMode);
};
}