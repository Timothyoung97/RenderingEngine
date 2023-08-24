#include "factory.h"

namespace tre {

Factory::Factory() {
	InitFactory();
}

void Factory::InitFactory() {

	//Create dxgiFactory

	CHECK_DX_ERROR(CreateDXGIFactory2(
		DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2)
	));

	//Create DXGI debug layer
	IDXGIDebug1* dxgiDebug = nullptr;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
	{
		dxgiDebug->EnableLeakTrackingForThread();
		IDXGIInfoQueue* dxgiInfoQueue = nullptr;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{

			dxgiInfoQueue->SetBreakOnSeverity(dxgi_debug_all, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
			dxgiInfoQueue->SetBreakOnSeverity(dxgi_debug_all, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(dxgi_debug_all, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true);

			dxgiInfoQueue->Release();
			dxgiDebug->Release();
		}
	}
}

Factory::~Factory() {
	dxgiFactory2->Release();
}

}