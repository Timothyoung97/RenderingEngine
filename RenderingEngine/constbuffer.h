#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include <vector>

#include "light.h"
#include "dxdebug.h"
#include "window.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace tre {

struct GlobalInfoStruct {
	XMFLOAT2 viewportDimension;
	XMFLOAT2 pad;
	XMFLOAT4 camPos;
	XMMATRIX viewProjection;
	XMMATRIX invViewProjection;
	XMMATRIX lightViewProjection[4];
	XMFLOAT4 planeIntervals;
	tre::Light light;
	UINT numOfPointLight;
	XMFLOAT2 shadowMapDimension;
	int csmDebugSwitch;
	int ssaoSwitch;
};

struct ViewProjectionStruct {
	XMMATRIX viewProjection;
};

struct ModelInfoStruct {
	XMMATRIX transformationLocal;
	XMMATRIX normalMatrix;
	XMFLOAT4 color;
	UINT isWithTexture;
	UINT hasNormalMap;
};

struct PointLightInfoStruct {
	UINT currPointLightIdx;
	XMFLOAT3 pad;
};

struct BatchInfoStruct {
	UINT batchOffset;
	XMFLOAT3 pad;
};

class CommonStructUtility {
public:
	// --Struct create functions-- //

	static GlobalInfoStruct createGlobalInfoStruct(
		const XMVECTOR& camPos,
		const XMMATRIX& viewProjection,
		const std::vector<XMMATRIX>& lightViewProjection,
		const XMFLOAT4& planeIntervals,
		const tre::Light& dirLight,
		int numOfPointLight,
		const XMFLOAT2& shadowMapDimension,
		int csmDebugSwitch,
		int ssaoSwtich
	);

	static ViewProjectionStruct createViewProjectionStruct(const XMMATRIX& viewProjection);
	static ModelInfoStruct createModelInfoStruct(const XMMATRIX& transformationLocal, const XMFLOAT4& color, UINT isWithTexture, UINT hasNormalMap);
	static PointLightInfoStruct createPointLightInfoStruct(int currPtLightIdx);
	static BatchInfoStruct createBatchInfoStruct(int batchOffset);
};

class Buffer {
public:
	
	// Creates an empty const buffer 
	static ID3D11Buffer* createConstBuffer(ID3D11Device* pDevice, UINT sizeOfBuffer);
	
	// Updates a const buffer with data from struct
	static void updateConstBufferData(ID3D11DeviceContext* pContext, ID3D11Buffer* pConstBuffer, void* pConstBufferInfo, UINT sizeOfConstBufferInfo);
};

}