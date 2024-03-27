#include "shader.h"

namespace tre {

void VertexShader::create(std::wstring fPath, ID3D11Device* device) {
	CHECK_DX_ERROR(D3DReadFileToBlob(
		fPath.c_str(), &pBlob
	));

	CHECK_DX_ERROR(device->CreateVertexShader(
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pShader
	));
}

void PixelShader::create(std::wstring fPath, ID3D11Device* device) {
	CHECK_DX_ERROR(D3DReadFileToBlob(
		fPath.c_str(), &pBlob
	));

	CHECK_DX_ERROR(device->CreatePixelShader(
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pShader
	));
}

void ComputeShader::create(std::wstring fPath, ID3D11Device* device) {
	CHECK_DX_ERROR(D3DReadFileToBlob(
		fPath.c_str(), &pBlob
	));

	CHECK_DX_ERROR(device->CreateComputeShader(
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pShader
	));
}

}