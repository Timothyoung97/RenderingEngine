#include "rendererBase.h"

namespace tre {

RendererBase::RendererBase() {
	this->init();
}

void RendererBase::init() {
	CHECK_DX_ERROR(pEngine->device->device.Get()->CreateDeferredContext(
		0u, contextD.GetAddressOf()
	));
}
}