#include <DirectXMath.h>

#include <vector>

#include "ssao.h"
#include "utility.h"
#include "dxdebug.h"
#include "maths.h"

using namespace DirectX;

namespace tre {

void SSAO::create(ID3D11Device* device, ID3D11DeviceContext* context) {
	_device = device;
	_context = context;

	std::vector<XMFLOAT3> ssaoKernal;
	for (int i = 0; i < 64; i++) {
		XMVECTOR newSample{
			tre::Utility::getRandomFloatRange(-1.f, 1.f),
			tre::Utility::getRandomFloatRange(-1.f, 1.f),
			tre::Utility::getRandomFloatRange(-1.f, 0.f)
		};

		newSample = XMVector3Normalize(newSample);

		float scale = (float)i / 64.0f;
		scale = tre::Maths::lerp(.1f, .1f, scale * scale);

		XMFLOAT3 sampleF;
		XMStoreFloat3(&sampleF, newSample * scale);

		ssaoKernal.push_back(sampleF);
	}

	D3D11_BUFFER_DESC ssaoKernalBufferDesc;
	ssaoKernalBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ssaoKernalBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ssaoKernalBufferDesc.CPUAccessFlags = 0u;
	ssaoKernalBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	ssaoKernalBufferDesc.ByteWidth = static_cast<UINT>(sizeof(XMFLOAT3) * ssaoKernal.size());
	ssaoKernalBufferDesc.StructureByteStride = static_cast<UINT>(sizeof(XMFLOAT3));

	D3D11_SUBRESOURCE_DATA ssaoKernalData = {};
	ssaoKernalData.pSysMem = ssaoKernal.data();

	CHECK_DX_ERROR(_device->CreateBuffer(
		&ssaoKernalBufferDesc, &ssaoKernalData, ssaoKernelBuffer.GetAddressOf()
	));
}
}