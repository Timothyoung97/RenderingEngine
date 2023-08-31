#include <d3dcompiler.h>

#include "dxdebug.h"
#include "shader.h"

namespace tre {

Shader::Shader(std::wstring vsFpath, std::wstring psFpath, ID3D11Device* device) {
	CHECK_DX_ERROR(D3DReadFileToBlob(
		vsFpath.c_str(), &pVSBlob
	));

	CHECK_DX_ERROR(D3DReadFileToBlob(
		psFpath.c_str(), &pPSBlob
	));

	CHECK_DX_ERROR(device->CreateVertexShader(
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &pVS
	));

	CHECK_DX_ERROR(device->CreatePixelShader(
		pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pPS
	));
}

}