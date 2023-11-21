#include "wireframeRenderer.h"
#include "utility.h"

namespace tre {

// Constructor: Create and allocate 
WireframeRenderer::WireframeRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : _device(pDevice), _context(pContext) {
	
	wireframeCube = tre::CubeMesh(_device);
	wireframeSphere = tre::SphereMesh(_device, 10, 10);
	
	std::wstring basePathWstr = tre::Utility::getBasePathWstr();
	_vertexShader.create(basePathWstr + L"shaders\\bin\\vertex_shader.bin", _device);
	_debugPixelShader.create(basePathWstr + L"shaders\\bin\\pixel_shader_debug.bin", _device);
}

void WireframeRenderer::draw(const Graphics& graphics, const std::vector<std::pair<Object*, Mesh*>> objQ, BoundVolumeEnum typeOfBound) {
	if (objQ.size() == 0) return;
	const char* name = ToString(tre::RENDER_MODE::WIREFRAME_M);
	MICROPROFILE_SCOPE_CSTR(name);
	PROFILE_GPU_SCOPED("Wireframe");

	// Select mesh to render based on bounding methods
	Mesh* meshToRender = nullptr;
	switch (typeOfBound) {
	case AABBBoundingBox:
		meshToRender = &wireframeCube;
		break;
	case RitterBoundingSphere: // fall through
	case NaiveBoundingSphere:
		meshToRender = &wireframeSphere;
		break;
	}

	// Configure Context
	{
		// Input Assembler
		UINT vertexStride = sizeof(Vertex), offset = 0u;
		_context->IASetInputLayout(graphics._inputLayout.vertLayout.Get());
		_context->IASetVertexBuffers(0, 1, meshToRender->pVertexBuffer.GetAddressOf(), &vertexStride, &offset);
		_context->IASetIndexBuffer(meshToRender->pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		_context->VSSetShader(_vertexShader.pShader.Get(), NULL, 0u);

		_context->RSSetViewports(1, &graphics._viewport.defaultViewport);
		_context->RSSetState(graphics._rasterizer.pRasterizerStateWireFrame.Get());

		_context->PSSetShader(_debugPixelShader.pShader.Get(), NULL, 0u);

		_context->OMSetBlendState(graphics._blendstate.transparency.Get(), NULL, 0xffffffff);
		_context->OMSetDepthStencilState(graphics._depthbuffer.pDSStateWithoutDepthT.Get(), 0);
		_context->OMSetRenderTargets(1, &graphics.currRenderTargetView, nullptr);
	}

	// Create empty const buffer and pre bind the constant buffer
	ID3D11Buffer* constBufferModelInfo = tre::ConstantBuffer::createConstBuffer(_device, sizeof(tre::ModelInfoStruct));
	{
		_context->VSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
		_context->PSSetConstantBuffers(1u, 1u, &constBufferModelInfo);
	}

	for (int i = 0; i < objQ.size(); i++) {

		tre::Object* currObj = objQ[i].first;

		for (int j = 0; j < currObj->pObjMeshes.size(); j++) {

			// Submit each object's data to const buffer
			{
				tre::ModelInfoStruct modelInfoStruct = tre::ConstantBuffer::createModelInfoStruct(currObj->_boundingVolumeTransformation, currObj->_boundingVolumeColor[j], 0u, 0u);
				tre::ConstantBuffer::updateConstBufferData(_context, constBufferModelInfo, &modelInfoStruct, sizeof(tre::ModelInfoStruct));
			}

			_context->DrawIndexed(meshToRender->indexSize, 0u, 0u);
		}
	}

	// clean up
	{
		constBufferModelInfo->Release();
	}
}
}