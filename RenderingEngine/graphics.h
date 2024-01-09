#pragma once

#include <vector>
#include <mutex>

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
#include "bloomBuffer.h"

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
	TONE_MAPPING_PASS,
	BLOOM_COMPUTE_PASS,
	HDR_COMPUTE_PASS
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
	case BLOOM_COMPUTE_PASS:				return "Bloom Compute Pass";
	case HDR_COMPUTE_PASS:					return "HDR Compute Pass";
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
	float bloomStrength = .2f;
	float bloomUpsampleRadius = .001f;
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
	Factory _factory;
	Swapchain _swapchain;

	// Pipeline
	Viewport _viewport;
	InputLayout _inputLayout;
	Rasterizer _rasterizer;
	Sampler _sampler;
	BlendState _blendstate;
	DepthBuffer _depthbuffer;
	
	// Buffers
	GBuffer _gBuffer;
	SSAO _ssao;
	HdrBuffer _hdrBuffer;
	BloomBuffer _bloomBuffer;
	InstanceBuffer _instanceBufferMainView;
	InstanceBuffer _instanceBufferCSM[4];
	InstanceBuffer _instanceBufferPointlights;
	InstanceBuffer _instanceBufferWireframes;

	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };

	std::vector<ID3D11Buffer*> bufferQueue;
	std::mutex bufferQueueMutex;

	// Misc
	GraphicsSetting setting;
	GraphicsStats stats;

	ComPtr<ID3D11RenderTargetView> currRenderTargetView;

	Graphics();

	void init();

	void clean();
	void clearSwapChainBuffer();

	void present();

	////////////////// Deprecated //////////////////
	//void configureStates(RENDER_MODE renderMode);
	//void draw(const std::vector<std::pair<Object*, Mesh*>> objQ, RENDER_MODE renderMode);
	//void fullscreenPass(tre::RENDER_MODE mode);
	//void instancedDraw(const std::vector<std::pair<Object*, Mesh*>>& objQ, RENDER_MODE renderMode);

};
}