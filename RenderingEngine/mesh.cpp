#include "mesh.h"

namespace tre {

CubeMesh::CubeMesh(ID3D11Device* device) {
	create(device);
}

void CubeMesh::create(ID3D11Device* device) {

	vector<Vertex> vertices;
	vector<uint16_t> indices;

	float unitLength = .5f;

	//Cube Vertices
	Vertex vertex[] = {
		// Back
		XMFLOAT3(unitLength, -unitLength, unitLength), XMFLOAT2(0, 1), // 0
		XMFLOAT3(-unitLength, -unitLength, unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(unitLength, unitLength, unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(-unitLength, unitLength, unitLength), XMFLOAT2(1, 0),

		// Right
		XMFLOAT3(unitLength, -unitLength, -unitLength), XMFLOAT2(0, 1), // 4
		XMFLOAT3(unitLength, -unitLength, unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(unitLength, unitLength, -unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(unitLength, unitLength, unitLength), XMFLOAT2(1, 0),

		// top
		XMFLOAT3(-unitLength, unitLength, -unitLength), XMFLOAT2(0, 1), // 8
		XMFLOAT3(unitLength, unitLength, -unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(-unitLength, unitLength, unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(unitLength, unitLength, unitLength), XMFLOAT2(1, 0),

		// Front
		XMFLOAT3(-unitLength, -unitLength, -unitLength), XMFLOAT2(0, 1), // 12
		XMFLOAT3(unitLength, -unitLength, -unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(-unitLength, unitLength, -unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(unitLength, unitLength, -unitLength), XMFLOAT2(1, 0),

		// Left
		XMFLOAT3(-unitLength, -unitLength, unitLength), XMFLOAT2(0, 1), // 16
		XMFLOAT3(-unitLength, -unitLength, -unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(-unitLength, unitLength, unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(-unitLength, unitLength, -unitLength), XMFLOAT2(1, 0),

		// bottom
		XMFLOAT3(-unitLength, -unitLength, unitLength), XMFLOAT2(0, 1), // 20
		XMFLOAT3(unitLength, -unitLength, unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(-unitLength, -unitLength, -unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(unitLength, -unitLength, -unitLength), XMFLOAT2(1, 0) // 23
	};

	vertices.assign(begin(vertex), end(vertex));

	//Cube Indices
	uint16_t index[] = {
		0, 1, 2, // back
		2, 1, 3,
		4, 5 ,6, // right
		6, 5, 7,
		8, 9, 10, // top
		10, 9, 11,
		12, 13, 14, // front
		14, 13, 15,
		16, 17, 18, // left
		18, 17, 19,
		20, 21, 22, // bottom
		22, 21, 23
	};

	indices.assign(begin(index), end(index));

	//Create vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0u;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vertexBufferDesc.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices.data();

	CHECK_DX_ERROR(device->CreateBuffer(
		&vertexBufferDesc, &vertexData, &pVertexBuffer
	));

	//Create index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0u;
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * indices.size());
	indexBufferDesc.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices.data();

	CHECK_DX_ERROR(device->CreateBuffer(
		&indexBufferDesc, &indexData, &pIndexBuffer
	));

	//Store index size
	indexSize = (int) indices.size();
}

SphereMesh::SphereMesh(ID3D11Device* device, int sectorC, int stackC) {
	create(device, sectorC, stackC);
}

void SphereMesh::create(ID3D11Device* device, int sectorC, int stackC) {

	vector<Vertex> vertices;
	vector<uint16_t> indices;

	//Sphere Properties
	float radius = .5f;
	int sectorCount = sectorC;
	int stackCount = stackC;
	float sectorStep = 2 * 180.0f / sectorCount;
	float stackStep = 180.0f / stackCount;

	float stackAngle = 90;
	float sectorAngle = 0;

	XMFLOAT3 sphereNormal;

	sphereNormal.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	sphereNormal.y = XMScalarSin(XMConvertToRadians(stackAngle));
	sphereNormal.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));

	//build north pole
	float v = 0;
	float u = 0;

	for (int i = 0; i < sectorCount; i++) {
		u = XMConvertToRadians(i * sectorStep + sectorStep / 2) / XM_2PI;
		vertices.push_back(Vertex(findCoordinate(sphereNormal, radius), XMFLOAT2(u, v)));
	}

	//build middle sec
	for (int i = 1; i < stackCount; i++) {
		stackAngle -= stackStep;
		v = XMConvertToRadians(i * stackStep) / XM_PI;
		for (int j = 0; j < sectorCount; j++) {
			sphereNormal.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
			sphereNormal.y = XMScalarSin(XMConvertToRadians(stackAngle));
			sphereNormal.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
			u = XMConvertToRadians(j * sectorStep) / XM_2PI;
			vertices.push_back(Vertex(findCoordinate(sphereNormal, radius), XMFLOAT2(u, v)));
			sectorAngle += sectorStep;
		}

		// one more vertice to map u to 1
		sectorAngle = 0;
		sphereNormal.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
		sphereNormal.y = XMScalarSin(XMConvertToRadians(stackAngle));
		sphereNormal.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
		vertices.push_back(Vertex(findCoordinate(sphereNormal, radius), XMFLOAT2(1, v)));
	}

	//build south pole
	sphereNormal.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(-90));
	sphereNormal.y = XMScalarSin(XMConvertToRadians(-90));
	sphereNormal.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(-90));

	for (int i = 0; i < sectorCount; i++) {
		u = XMConvertToRadians(i * sectorStep + sectorStep / 2) / XM_2PI;
		vertices.push_back(Vertex(findCoordinate(sphereNormal, radius), XMFLOAT2(u, 1)));
	}

	//Build north pole indices
	for (int i = 0; i < sectorCount; i++) {
		int nextStackIdx = i + sectorCount;
		indices.push_back(i);
		indices.push_back(nextStackIdx);
		indices.push_back(nextStackIdx + 1);
	}

	// Build middle
	// 
	// k1 - k1 + 1
	// | a / |
	// |  /  |
	// | / b |
	// k2 - k2 + 1
	int upperStackIdx = sectorCount; // upperStackIdx = 0 + sectorCount
	int lowerStackIdx = upperStackIdx + sectorCount + 1; // lowerStackIdx = upperStackIdx + sectorCount + 1

	for (int i = 1; i < stackCount - 1; i++) {

		for (int j = 0; j < sectorCount; j++) {

			// triangle a
			indices.push_back(upperStackIdx);
			indices.push_back(lowerStackIdx);
			indices.push_back(upperStackIdx + 1);

			//triangle b
			indices.push_back(upperStackIdx + 1);
			indices.push_back(lowerStackIdx);
			indices.push_back(lowerStackIdx + 1);

			upperStackIdx++;
			lowerStackIdx++;
		}
		upperStackIdx++;
		lowerStackIdx++;
	}

	// Build south pole indices
	int southPoleIdx = (int) vertices.size() - sectorCount;

	for (int i = 0; i < sectorCount; i++) {
		int upperIdx = southPoleIdx - sectorCount - 1;
		indices.push_back(upperIdx);
		indices.push_back(southPoleIdx);
		indices.push_back(upperIdx + 1);
		southPoleIdx++;
	}

	//Create vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0u;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vertexBufferDesc.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices.data();

	CHECK_DX_ERROR(device->CreateBuffer(
		&vertexBufferDesc, &vertexData, &pVertexBuffer
	));

	//Create index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0u;
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint16_t) * indices.size());
	indexBufferDesc.StructureByteStride = 0u;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices.data();

	CHECK_DX_ERROR(device->CreateBuffer(
		&indexBufferDesc, &indexData, &pIndexBuffer
	));

	//Store index size
	indexSize = (int) indices.size();
}

XMFLOAT3 SphereMesh::findCoordinate(XMFLOAT3 unitVector, float radius) {
	return XMFLOAT3(unitVector.x * radius, unitVector.y * radius, unitVector.z * radius);
}

}