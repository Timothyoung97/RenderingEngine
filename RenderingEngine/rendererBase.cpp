#include "rendererBase.h"

#include "engine.h"
#include "dxdebug.h"

extern tre::Engine* pEngine;

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