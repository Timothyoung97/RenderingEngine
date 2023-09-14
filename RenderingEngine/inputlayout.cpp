#include "inputlayout.h"

#include "dxdebug.h"

namespace tre {

InputLayout::InputLayout(ID3D11Device* device, VertexShader* vertShader) {
	// Create input layout
	CHECK_DX_ERROR(device->CreateInputLayout(
		layout, numOfInputElement, vertShader->pBlob.Get()->GetBufferPointer(), vertShader->pBlob.Get()->GetBufferSize(), vertLayout.GetAddressOf()
	));
}
}