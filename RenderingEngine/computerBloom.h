#pragma once

#include <DirectXMath.h>

#include "rendererBase.h"
#include "graphics.h"

using namespace DirectX;

namespace tre {

struct BloomConstBufferStruct {
    XMINT2 srcViewportDimension;
    XMINT2 destViewportDimension;
    float sampleRadius;
    XMFLOAT3 pad;
};

class ComputerBloom : public RendererBase {
public:
    ComputeShader computeShaderDownsample;
    ComputeShader computeShaderUpsample;

    int operationCount = 5;

    ComputerBloom();

	void init();

    void downsample(Graphics& graphics);
    void singleDownsample(Graphics& graphics, ID3D11Resource* pSampleTexture, ID3D11Resource* pDownsampleTexture, XMINT2 sampleViewDimension);

    void upsample(Graphics& graphics);
    void singleUpsample(Graphics& graphics, ID3D11Resource* pSampleTexture, ID3D11Resource* pDownsampleTexture, XMINT2 sampleViewDimension);

	void compute(Graphics& graphics);
};
}