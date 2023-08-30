#include "constbuffer.h"

namespace tre {

ConstantBuffer::ConstantBuffer() {
	constantBufferDescCam.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescCam.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDescCam.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDescCam.MiscFlags = 0u;
	constantBufferDescCam.ByteWidth = sizeof(constBufferShaderRescCam);
	constantBufferDescCam.StructureByteStride = 0u;	
	
	constantBufferDescModel.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDescModel.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDescModel.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDescModel.MiscFlags = 0u;
	constantBufferDescModel.ByteWidth = sizeof(constBufferShaderRescModel);
	constantBufferDescModel.StructureByteStride = 0u;
}

}