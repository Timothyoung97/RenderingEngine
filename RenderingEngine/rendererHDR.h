#include <d3d11.h>
#include "graphics.h"
#include "constbuffer.h"

using namespace DirectX;

namespace tre {
class RendererHDR {
public:
	ID3D11Device* _device;
	ID3D11DeviceContext* _context;

	ComputeShader _computeShaderLuminancehistogram;
	ComputeShader _computeShaderLuminanceAverage;

	VertexShader _vertexShaderFullscreenQuad;
	PixelShader _hdrPixelShader;

	RendererHDR(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	static HDRStruct createHDRStruct(float middleGrey);
	static LuminanceStruct createLuminanceStruct(const XMFLOAT2& luminance, float timeCoeff);

	void setConstBufferLuminSetting(Graphics& graphics);
	void setConstBufferHDR(Graphics& graphics);

	void dispatchHistogram(const Graphics& graphics);
	void dispatchAverage(const Graphics& graphics);
	void fullscreenPass(const Graphics& graphics);
};
}