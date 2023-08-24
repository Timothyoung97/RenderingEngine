#include "device.h"

using Microsoft::WRL::ComPtr;

namespace tre {

Device::Device() {
	InitDXDevice();
};

void Device::InitDXDevice() {

	UINT creationFlags = D3D11_CREATE_DEVICE_DEBUG;

	D3D_FEATURE_LEVEL featureLevel;

	CHECK_DX_ERROR( D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, nullptr, 0, D3D11_SDK_VERSION, &device, &featureLevel, &context
	));

	//Create D3D11 debug layer
	ComPtr<ID3D11Debug> d3dDebug;

	CHECK_DX_ERROR( device->QueryInterface(
		__uuidof(ID3D11Debug), &d3dDebug
	));
	
	ComPtr<ID3D11InfoQueue> d3dInfoQueue;

	CHECK_DX_ERROR( d3dDebug->QueryInterface(
		__uuidof(ID3D11InfoQueue), &d3dInfoQueue
	));

	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
};
}