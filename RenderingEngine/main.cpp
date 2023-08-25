#define SDL_MAIN_HANDLED

#include <dxgi1_4.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <vector>

//Custom Header
#include "timer.h"
#include "window.h"
#include "input.h"
#include "dxdebug.h"
#include "device.h"
#include "factory.h"
#include "swapchain.h"

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

using namespace DirectX;

struct constBufferShaderResc {
	XMMATRIX transformation;
	XMMATRIX viewProjection;
	//XMFLOAT4 rgbaColor;
};

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

//Input Layout
D3D11_INPUT_ELEMENT_DESC layout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
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

	// Colors
	XMFLOAT4 colors[10] = {
		{1.0f, .0f, .0f, 1.0f},
		{.0f, 1.0f, .0f, 1.0f},
		{.0f, .0f, 1.0f, 1.0f},
		{1.0f, 1.0f, .0f, 1.0f},
		{1.0f, .0f, 1.0f, 1.0f},
		{.0f, 1.0f, 1.0f, 1.0f},
		{.5f, .0f, .5f, 1.0f},
		{1.0f, .0f, .3f, 1.0f},
		{1.0f, .0f, .2f, 1.0f},
		{.0f, .7f, .0f, 1.0f},
	};

	////Cube Vertices
	//Vertex cubeVertex[] = {
	//	XMFLOAT3(-.5, .5, -.5), {1.0f, .0f, .0f, 1.0f},
	//	XMFLOAT3(-.5, .5, .5), {.0f, 1.0f, .0f, 1.0f},
	//	XMFLOAT3(.5, .5, .5), {.0f, .0f, 1.0f, 1.0f},
	//	XMFLOAT3(.5, .5, -.5), {1.0f, 1.0f, .0f, 1.0f},
	//	XMFLOAT3(.5, -.5, -.5), {1.0f, .0f, 1.0f, 1.0f},
	//	XMFLOAT3(.5, -.5, .5), {.0f, 1.0f, 1.0f, 1.0f},
	//	XMFLOAT3(-.5, -.5, .5), {.5f, .5f, .5f, 1.0f},
	//	XMFLOAT3(-.5, -.5, -.5), {.75f, .5f, .75f, 1.0f}
	//};

	////Cube Indices
	//uint16_t indices[] = {
	//	0, 7, 3, // -z
	//	3, 7, 4,
	//	1, 0, 2, // +y
	//	2, 0, 3,
	//	3, 4, 2, // +x
	//	2, 4, 5,
	//	7, 6, 4, // -y
	//	4, 6, 5,
	//	2, 5, 1, // +z
	//	1, 5, 6,
	//	1, 6, 0, // -x
	//	0, 6, 7
	//};

	//const UINT numOfIndices = ARRAYSIZE(indices);

	float stackAngle = 90;
	float sectorAngle = 0;

	XMFLOAT3 sphereNormal;

	sphereNormal.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	sphereNormal.y = XMScalarSin(XMConvertToRadians(stackAngle));
	sphereNormal.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));

	//Sphere Properties
	float radius = .5f;
	int sectorCount = 10;
	int stackCount = 10;

	float sectorStep = 2 * 180 / sectorCount;
	float stackStep = 180 / stackCount;
	int colorIdx = 0;

	//Sphere Vertices
	std::vector<Vertex> sphereVertices;

	sphereVertices.push_back(Vertex(sphereNormal, colors[colorIdx]));

	for (int i = 1; i < stackCount; i++) {
		colorIdx = colorIdx == 0 ? 9 : colorIdx - 1;
		stackAngle -= stackStep;
		for (int j = 0; j < sectorCount; j++) {
			sphereNormal.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
			sphereNormal.y = XMScalarSin(XMConvertToRadians(stackAngle));
			sphereNormal.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
			sphereVertices.push_back(Vertex(sphereNormal, colors[colorIdx]));
			sectorAngle += sectorStep;
		}
		sectorAngle = 0;
	}

	sphereNormal.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(-90));
	sphereNormal.y = XMScalarSin(XMConvertToRadians(-90));
	sphereNormal.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(-90));
	sphereVertices.push_back(Vertex(sphereNormal, colors[colorIdx]));

	//Sphere indices
	std::vector<uint16_t> sphereIndices;

	//Build north indices
	int northPoleIdx = 0;
	int nextIdx = 1;
	for (int i = 0; i < sectorCount; i++) {
		
		if (i == sectorCount - 1) {
			sphereIndices.push_back(northPoleIdx);
			sphereIndices.push_back(nextIdx);
			sphereIndices.push_back(1);
			break;
		}
		
		sphereIndices.push_back(northPoleIdx);
		sphereIndices.push_back(nextIdx);
		sphereIndices.push_back(nextIdx + 1);
		nextIdx++;
	}

	// Build middle
	// 
	// k1 - k1 + 1
	// | a / |
	// |  /  |
	// | / b |
	// k2 - k2 + 1
	int upperStackIdx = 1;
	int lowerStackIdx = upperStackIdx + sectorCount;
	
	for (int i = 1; i < stackCount - 1; i++) {

		for (int j = 0; j < sectorCount; j++) {
			
			if (j == sectorCount - 1) {
				// triangle a
				sphereIndices.push_back(upperStackIdx);
				sphereIndices.push_back(lowerStackIdx);
				sphereIndices.push_back(upperStackIdx - sectorCount + 1);

				//triangle b
				sphereIndices.push_back(upperStackIdx - sectorCount + 1);
				sphereIndices.push_back(lowerStackIdx);
				sphereIndices.push_back(lowerStackIdx - sectorCount + 1);

				upperStackIdx++;
				lowerStackIdx++;

				break;
			}

			// triangle a
			sphereIndices.push_back(upperStackIdx);
			sphereIndices.push_back(lowerStackIdx);
			sphereIndices.push_back(upperStackIdx + 1);

			//triangle b
			sphereIndices.push_back(upperStackIdx + 1);
			sphereIndices.push_back(lowerStackIdx);
			sphereIndices.push_back(lowerStackIdx + 1);

			upperStackIdx++;
			lowerStackIdx++;

		}
	}

	// Build bottom
	int southPoleIdx = sphereVertices.size() - 1;
	lowerStackIdx = sphereVertices.size() - 1 - sectorCount;

	for (int i = 0; i < sectorCount; i++) {

		if (i == sectorCount - 1) {
			sphereIndices.push_back(lowerStackIdx);
			sphereIndices.push_back(southPoleIdx);
			sphereIndices.push_back(lowerStackIdx - sectorCount + 1);
			break;
		}

		sphereIndices.push_back(lowerStackIdx);
		sphereIndices.push_back(southPoleIdx);
		sphereIndices.push_back(lowerStackIdx + 1);
		lowerStackIdx++;
	}

	UINT numOfSphereIndices = sphereIndices.size();

	//Create index buffer
	ID3D11Buffer* pIndexBuffer;

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0u;
	//indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.ByteWidth = sizeof(uint16_t) * numOfSphereIndices;
	indexBufferDesc.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA indexData = {};
	//indexData.pSysMem = &indices;
	indexData.pSysMem = sphereIndices.data();

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
	//vertexBufferDesc.ByteWidth = sizeof(cubeVertex);
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * sphereVertices.size();
	vertexBufferDesc.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	//vertexData.pSysMem = &cubeVertex;
	vertexData.pSysMem = sphereVertices.data();

	CHECK_DX_ERROR(deviceAndContext.device->CreateBuffer(
		&vertexBufferDesc, &vertexData, &pVertexBuffer
	));

	//Set vertex buffer
	UINT vertexStride = sizeof(Vertex);
	UINT offset = 0;
	deviceAndContext.context->IASetVertexBuffers(0, 1, &pVertexBuffer, &vertexStride, &offset);

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
		} else if (input.keyState[SDL_SCANCODE_W]) {
			camPositionF.z += cameraMoveSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_S]) {
			camPositionF.z -= cameraMoveSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_D]) {
			camPositionF.x += cameraMoveSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_A]) {
			camPositionF.x -= cameraMoveSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_Q]) {
			camPositionF.y -= cameraMoveSpeed * deltaTime;
		} else if (input.keyState[SDL_SCANCODE_E]) {
			camPositionF.y += cameraMoveSpeed * deltaTime;
		} else if (input.mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) {

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

		camPositionV = XMLoadFloat4(&camPositionF);

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
		deviceAndContext.context->DrawIndexed(numOfSphereIndices, 0, 0);

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
