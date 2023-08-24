#include "device.h"

namespace tre {

Device::Device() : device(nullptr), context(nullptr) {
	InitDXDevice();
};

void Device::InitDXDevice() {

	UINT creationFlags = D3D11_CREATE_DEVICE_DEBUG;

	D3D_FEATURE_LEVEL featureLevel;

	CHECK_DX_ERROR( D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, nullptr, 0, D3D11_SDK_VERSION, &device, &featureLevel, &context
	));

	//Create D3D11 debug layer
	ID3D11Debug* d3dDebug = nullptr;

	CHECK_DX_ERROR( device->QueryInterface(
		__uuidof(ID3D11Debug), (void**)&d3dDebug
	));
	
	ID3D11InfoQueue* d3dInfoQueue = nullptr;

	CHECK_DX_ERROR( d3dDebug->QueryInterface(
		__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue
	));

	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);

	d3dInfoQueue->Release();
	d3dDebug->Release();
};

Device::~Device() {
	context->Release();
	device->Release();
};
}