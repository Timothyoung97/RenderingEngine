#include "object3d.h"

namespace tre {

Cube3d::Cube3d(float unitLength) {
	create(XMFLOAT3(.0f, .0f, .0f), unitLength);
}

Cube3d::Cube3d(XMFLOAT3 origin, float unitLength) {
	create(origin, unitLength);
}

void Cube3d::create(XMFLOAT3 origin, float unitLength) {
	
	float x, y, z;
	x = origin.x;
	y = origin.y;
	z = origin.z;

	//Cube Vertices
	Vertex vertex[] = {
		// Back
		XMFLOAT3(x + unitLength, y - unitLength, z + unitLength), XMFLOAT2(0, 1), // 0
		XMFLOAT3(x - unitLength, y - unitLength, z + unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(x + unitLength, y + unitLength, z + unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(x - unitLength, y + unitLength, z + unitLength), XMFLOAT2(1, 0),

		// Right
		XMFLOAT3(x + unitLength, y - unitLength, z - unitLength), XMFLOAT2(0, 1), // 4
		XMFLOAT3(x + unitLength, y - unitLength, z + unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(x + unitLength, y + unitLength, z - unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(x + unitLength, y + unitLength, z + unitLength), XMFLOAT2(1, 0),

		// top
		XMFLOAT3(x - unitLength, y + unitLength, z - unitLength), XMFLOAT2(0, 1), // 8
		XMFLOAT3(x + unitLength, y + unitLength, z - unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(x - unitLength, y + unitLength, z + unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(x + unitLength, y + unitLength, z + unitLength), XMFLOAT2(1, 0),

		// Front
		XMFLOAT3(x - unitLength, y - unitLength, z - unitLength), XMFLOAT2(0, 1), // 12
		XMFLOAT3(x + unitLength, y - unitLength, z - unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(x - unitLength, y + unitLength, z - unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(x + unitLength, y + unitLength, z - unitLength), XMFLOAT2(1, 0),

		// Left
		XMFLOAT3(x - unitLength, y - unitLength, z + unitLength), XMFLOAT2(0, 1), // 16
		XMFLOAT3(x - unitLength, y - unitLength, z - unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(x - unitLength, y + unitLength, z + unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(x - unitLength, y + unitLength, z - unitLength), XMFLOAT2(1, 0),

		// bottom
		XMFLOAT3(x - unitLength, y - unitLength, z + unitLength), XMFLOAT2(0, 1), // 20
		XMFLOAT3(x + unitLength, y - unitLength, z + unitLength), XMFLOAT2(1, 1),
		XMFLOAT3(x - unitLength, y - unitLength, z - unitLength), XMFLOAT2(0, 0),
		XMFLOAT3(x + unitLength, y - unitLength, z - unitLength), XMFLOAT2(1, 0) // 23
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
}

Sphere3d::Sphere3d(float r, int sectorC, int stackC) {
	create(XMFLOAT3(.0f, .0f, .0f), r, sectorC, stackC);
}

Sphere3d::Sphere3d(XMFLOAT3 origin, float r, int sectorC, int stackC) {
	create(origin, r, sectorC, stackC);
}

void Sphere3d::create(XMFLOAT3 origin, float r, int sectorC, int stackC) {

	//Sphere Properties
	float radius = r;
	int sectorCount = sectorC;
	int stackCount = stackC;
	float sectorStep = 2 * 180 / sectorCount;
	float stackStep = 180 / stackCount;

	float stackAngle = 90;
	float sectorAngle = 0;

	XMFLOAT3 sphereNormal(origin.x, origin.y, origin.z);

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
	int southPoleIdx = vertices.size() - sectorCount;

	for (int i = 0; i < sectorCount; i++) {
		int upperIdx = southPoleIdx - sectorCount - 1;
		indices.push_back(upperIdx);
		indices.push_back(southPoleIdx);
		indices.push_back(upperIdx + 1);
		southPoleIdx++;
	}
}

XMFLOAT3 Sphere3d::findCoordinate(XMFLOAT3 unitVector, float radius) {
	return XMFLOAT3(unitVector.x * radius, unitVector.y * radius, unitVector.z * radius);
}

}