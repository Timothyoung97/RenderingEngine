#include "factory.h"

using Microsoft::WRL::ComPtr;

namespace tre {

Factory::Factory() {
	InitFactory();
}

void Factory::InitFactory() {

	//Create dxgiFactory
	CHECK_DX_ERROR(CreateDXGIFactory2(
		DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory2), &dxgiFactory2
	));

	//Create DXGI debug layer
	ComPtr<IDXGIDebug1> dxgiDebug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
	{
		dxgiDebug->EnableLeakTrackingForThread();
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue = nullptr;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{
			dxgiInfoQueue->SetBreakOnSeverity(dxgi_debug_all, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
			dxgiInfoQueue->SetBreakOnSeverity(dxgi_debug_all, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(dxgi_debug_all, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true);
		}
	}
}

}