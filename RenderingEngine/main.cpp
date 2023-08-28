#define SDL_MAIN_HANDLED

#include <dxgi1_4.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl/client.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vector>

//Custom Header
#include "timer.h"
#include "window.h"
#include "input.h"
#include "dxdebug.h"
#include "device.h"
#include "factory.h"
#include "swapchain.h"
#include "object3d.h"

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct constBufferShaderResc {
	XMMATRIX transformation;
	XMMATRIX viewProjection;
	//XMFLOAT4 rgbaColor;
};

//Input Layout
D3D11_INPUT_ELEMENT_DESC layout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const UINT numOfInputElement = ARRAYSIZE(layout);

int main()
{
	//Create Window
	tre::Window window("RenderingEngine", SCREEN_WIDTH, SCREEN_HEIGHT);

	//Create Device 
	tre::Device deviceAndContext;

	//Create dxgiFactory
	tre::Factory factory;

	//Create SwapChain
	tre::Swapchain swapchain;
	swapchain.DescSwapchain(SCREEN_WIDTH, SCREEN_HEIGHT);
	swapchain.InitSwapchainViaHwnd(factory.dxgiFactory2, deviceAndContext.device, window.getWindowHandle());

	//Load pre-compiled shaders
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;

	CHECK_DX_ERROR( D3DReadFileToBlob(
		L"../RenderingEngine/shaders/vertex_shader.bin", &pVSBlob
	));

	CHECK_DX_ERROR( D3DReadFileToBlob(
		L"../RenderingEngine/shaders/pixel_shader.bin", &pPSBlob
	));

	ID3D11VertexShader* vertex_shader_ptr = nullptr;
	ID3D11PixelShader* pixel_shader_ptr = nullptr;

	CHECK_DX_ERROR(deviceAndContext.device->CreateVertexShader(
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &vertex_shader_ptr
	));

	CHECK_DX_ERROR(deviceAndContext.device->CreatePixelShader(
		pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pixel_shader_ptr
	));

	deviceAndContext.context->VSSetShader(vertex_shader_ptr, NULL, 0u);
	deviceAndContext.context->PSSetShader(pixel_shader_ptr, NULL, 0u);

	// 3D object
	tre::Sphere3d cube(.5f, 10, 10);

	//Create index buffer
	ID3D11Buffer* pIndexBuffer;

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0u;
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * cube.indices.size());
	//indexBufferDesc.ByteWidth = sizeof(uint16_t) * numOfSphereIndices;
	indexBufferDesc.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = cube.indices.data();
	//indexData.pSysMem = sphereIndices.data();

	CHECK_DX_ERROR(deviceAndContext.device->CreateBuffer(
		&indexBufferDesc, &indexData, &pIndexBuffer
	));

	//Set index buffer
	deviceAndContext.context->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	//Create vertex buffer
	ID3D11Buffer* pVertexBuffer;

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0u;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * cube.vertices.size());
	//vertexBufferDesc.ByteWidth = sizeof(Vertex) * sphereVertices.size();
	vertexBufferDesc.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = cube.vertices.data();
	//vertexData.pSysMem = sphereVertices.data();

	CHECK_DX_ERROR(deviceAndContext.device->CreateBuffer(
		&vertexBufferDesc, &vertexData, &pVertexBuffer
	));

	//Set vertex buffer
	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;
	deviceAndContext.context->IASetVertexBuffers(0, 1, &pVertexBuffer, &vertexStride, &offset);

	// load image using stb
	int imgWidth, imgHeight, imgChannels, desiredChannels = 4;
	unsigned char* img = stbi_load("../UV_image.jpg", &imgWidth, &imgHeight, &imgChannels, desiredChannels);
	if (img == NULL) {
		printf("Error loading image");
	}
	printf("Img width: %d, Img height: %d, Img channels: %d\n", imgWidth, imgHeight, imgChannels);

	// Create texture
	D3D11_TEXTURE2D_DESC texture2dDesc;
	texture2dDesc.Width = imgWidth;
	texture2dDesc.Height = imgHeight;
	texture2dDesc.MipLevels = 1;
	texture2dDesc.ArraySize = 1;
	texture2dDesc.SampleDesc.Count = 1;
	texture2dDesc.SampleDesc.Quality = 0;
	texture2dDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2dDesc.Usage = D3D11_USAGE_DYNAMIC;
	texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture2dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texture2dDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA textureData = {};
	textureData.pSysMem = img;
	textureData.SysMemPitch = imgWidth * 4;
	textureData.SysMemSlicePitch = textureData.SysMemPitch * imgHeight;

	ComPtr<ID3D11Texture2D> pTexture;

	CHECK_DX_ERROR(deviceAndContext.device->CreateTexture2D(
		&texture2dDesc, &textureData, pTexture.GetAddressOf()
	));

	STBI_FREE(img);

	// Create Shader Resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc;
	shaderResViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D = D3D11_TEX2D_SRV(0, -1);

	ComPtr<ID3D11ShaderResourceView> pShaderResView;

	CHECK_DX_ERROR(deviceAndContext.device->CreateShaderResourceView(
		pTexture.Get(), &shaderResViewDesc, pShaderResView.GetAddressOf()
	));

	// Create sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	ComPtr<ID3D11SamplerState> pSamplerState;

	CHECK_DX_ERROR(deviceAndContext.device->CreateSamplerState(
		&samplerDesc, pSamplerState.GetAddressOf()
	));

	// set shader resc view and sampler
	deviceAndContext.context->PSSetShaderResources(0, 1, pShaderResView.GetAddressOf());
	deviceAndContext.context->PSSetSamplers(0, 1, pSamplerState.GetAddressOf());

	// Create input layout
	ID3D11InputLayout* vertLayout;

	CHECK_DX_ERROR(deviceAndContext.device->CreateInputLayout(
		layout, numOfInputElement, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &vertLayout
	));

	//Create rasterizer buffer
	ID3D11RasterizerState* pRasterizerState;

	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;

	CHECK_DX_ERROR(deviceAndContext.device->CreateRasterizerState(
		&rasterizerDesc, &pRasterizerState
	));
	
	//Set rasterizer state
	deviceAndContext.context->RSSetState(pRasterizerState);

	// Set input layout
	deviceAndContext.context->IASetInputLayout( vertLayout );

	// Set topology
	deviceAndContext.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create Viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	//Set Viewport
	deviceAndContext.context->RSSetViewports(1, &viewport);

	// temp tranformation data
	float offsetX = .0f;
	float offsetY = .0f;
	float translateSpeed = .001f;

	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float scaleSpeed = .001f;

	float rotateZ = .0f;
	float rotateSpeed = .1f;

	//Input Handler
	tre::Input input;
	
	//Colors
	float bgColor[4] = { .5f, .5f, .5f, 1.0f };
	
	XMFLOAT4 triangleColor[3] = {
		{1, 0, 0, 1},
		{0, 1, 0, 1},
		{0, 0, 1, 1}
	};

	int currTriColor = 0;

	//Delta Time between frame
	double deltaTime = 0;

	// Camera's properties
	XMVECTOR defaultUpV = XMVectorSet(.0f, 1.0f, .0f, .0f);
	
	XMVECTOR camPositionV = XMVectorSet(.0f, .0f, -3.0f, .0f);
	XMVECTOR camUpV = XMVectorSet(.0f, 1.0f, .0f, .0f);
	XMVECTOR camRightV = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	float yaw = 90;
	float pitch = 0;

	XMFLOAT3 directionF;
	directionF.x = XMScalarCos(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));
	directionF.y = XMScalarSin(XMConvertToRadians(pitch));
	directionF.z = XMScalarSin(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));

	XMVECTOR directionV;
	directionV = XMVector3Normalize(XMLoadFloat3(&directionF));

	float cameraMoveSpeed = .001f;
	float cameraRotateSpeed = .01f;

	// Camera View Matrix
	XMMATRIX camView;

	// Projection Matrix
	XMMATRIX camProjection;
	camProjection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(SCREEN_WIDTH / SCREEN_HEIGHT), 1.0f, 1000.0f);

	// main loop
	while (!input.shouldQuit())
	{
		tre::Timer timer;

		// Update keyboard event
		input.updateInputEvent();

		XMFLOAT4 camPositionF;
		XMStoreFloat4(&camPositionF, camPositionV);

		// Control
		if (input.keyState[SDL_SCANCODE_LEFT]) {
			offsetX -= translateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_RIGHT]) {
			offsetX += translateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_UP]) {
			offsetY += translateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_DOWN]) {
			offsetY -= translateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_PAGEUP]) {
			scaleX += scaleSpeed * deltaTime;
			scaleY += scaleSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_PAGEDOWN]) {
			scaleX -= scaleSpeed * deltaTime;
			scaleY -= scaleSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_Z]) {
			rotateZ += rotateSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_1]) {
			currTriColor = 0;
		} else if (input.keyState[SDL_SCANCODE_2]) {
			currTriColor = 1;
		} else if (input.keyState[SDL_SCANCODE_3]) {
			currTriColor = 2;
		} else if (input.keyState[SDL_SCANCODE_W]) { // control camera movement
			camPositionV = XMVectorAdd(camPositionV, directionV * cameraMoveSpeed * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_S]) {
			camPositionV = XMVectorAdd(camPositionV, -directionV * cameraMoveSpeed * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_D]) {
			camPositionV = XMVectorAdd(camPositionV, -camRightV * cameraMoveSpeed * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_A]) {
			camPositionV = XMVectorAdd(camPositionV, camRightV * cameraMoveSpeed * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_Q]) {
			camPositionV = XMVectorAdd(camPositionV, defaultUpV * cameraMoveSpeed * deltaTime);
		} else if (input.keyState[SDL_SCANCODE_E]) {
			camPositionV = XMVectorAdd(camPositionV, -defaultUpV * cameraMoveSpeed * deltaTime);
		} else if (input.mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) { // control camera angle

			yaw -= input.deltaDisplacement.x * cameraRotateSpeed;
			pitch -= input.deltaDisplacement.y * cameraRotateSpeed;

			directionF.x = XMScalarCos(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));
			directionF.y = XMScalarSin(XMConvertToRadians(pitch));
			directionF.z = XMScalarSin(XMConvertToRadians(yaw)) * XMScalarCos(XMConvertToRadians(pitch));

			directionV = XMVector3Normalize(XMLoadFloat3(&directionF));
		}

		// Update camera
		camRightV = XMVector3Normalize(XMVector3Cross(directionV, defaultUpV));
		camUpV = XMVector3Normalize(XMVector3Cross(camRightV, directionV));

		camView = XMMatrixLookAtLH(camPositionV, camPositionV + directionV, camUpV);

		// model matrix = scale -> rotate -> translate
		XMMATRIX tf_matrix = XMMatrixMultiply(
			XMMatrixScaling(scaleX, scaleY, 1),
			XMMatrixMultiply(
				XMMatrixRotationZ(XMConvertToRadians(rotateZ)),
				XMMatrixTranslation(offsetX, offsetY, 0)
			)
		);

		// Constant Buffer Shader Resource
		constBufferShaderResc cbsr;
		cbsr.transformation = tf_matrix;
		cbsr.viewProjection = XMMatrixMultiply(camView, camProjection);
		//cbsr.rgbaColor = triangleColor[currTriColor];

		// Constant buffer
		D3D11_BUFFER_DESC constantBufferDesc;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantBufferDesc.MiscFlags = 0u;
		constantBufferDesc.ByteWidth = sizeof(cbsr);
		constantBufferDesc.StructureByteStride = 0u;

		ID3D11Buffer* pConstBuffer;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &cbsr;
		CHECK_DX_ERROR(deviceAndContext.device->CreateBuffer(
			&constantBufferDesc, &csd, &pConstBuffer
		));

		deviceAndContext.context->VSSetConstantBuffers(0u, 1u, &pConstBuffer);
		deviceAndContext.context->PSSetConstantBuffers(0u, 1u, &pConstBuffer);

		// Alternating buffers
		int currBackBuffer = static_cast<int>(swapchain.mainSwapchain->GetCurrentBackBufferIndex());

		ID3D11Texture2D* backBuffer = nullptr;

		CHECK_DX_ERROR(swapchain.mainSwapchain->GetBuffer(
			currBackBuffer, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer
		));

		// Create render target view
		ID3D11RenderTargetView* renderTargetView = nullptr;

		CHECK_DX_ERROR(deviceAndContext.device->CreateRenderTargetView(
			backBuffer, NULL, &renderTargetView
		));

		deviceAndContext.context->OMSetRenderTargets(1, &renderTargetView, nullptr);
		
		deviceAndContext.context->ClearRenderTargetView(renderTargetView, bgColor);

		//device.context->DrawIndexed(numOfIndices, 0, 0);
		deviceAndContext.context->DrawIndexed(cube.indices.size(), 0, 0);

		CHECK_DX_ERROR(swapchain.mainSwapchain->Present( 0, 0) );

		while (timer.getDeltaTime() < 1000.0 / 30) {
		}

		deltaTime = timer.getDeltaTime();

		pConstBuffer->Release();

	}

	//Cleanup
	vertLayout->Release();

	return 0;
}
