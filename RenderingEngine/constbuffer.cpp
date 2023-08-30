#include "constbuffer.h"

namespace tre {

ConstantBuffer::ConstantBuffer() {
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0u;
	constantBufferDesc.ByteWidth = sizeof(constBufferShaderResc);
	constantBufferDesc.StructureByteStride = 0u;
}

}