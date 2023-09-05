#include <d3dcompiler.h>

#include "dxdebug.h"
#include "shader.h"

namespace tre {

VertexShader::VertexShader(std::wstring fPath, ID3D11Device* device) {
	CHECK_DX_ERROR(D3DReadFileToBlob(
		fPath.c_str(), &pBlob
	));

	CHECK_DX_ERROR(device->CreateVertexShader(
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pShader
	));
}

PixelShader::PixelShader(std::wstring fPath, ID3D11Device* device) {
	CHECK_DX_ERROR(D3DReadFileToBlob(
		fPath.c_str(), &pBlob
	));

	CHECK_DX_ERROR(device->CreatePixelShader(
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pShader
	));
}

}