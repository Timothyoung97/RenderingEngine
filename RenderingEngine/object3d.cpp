#include "object3d.h"

namespace tre {
Cube3d::Cube3d(float unitLength) {

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
}

Sphere3d::Sphere3d(float r, int sectorC, int stackC) {

	float stackAngle = 90;
	float sectorAngle = 0;

	XMFLOAT3 sphereNormal;

	sphereNormal.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	sphereNormal.y = XMScalarSin(XMConvertToRadians(stackAngle));
	sphereNormal.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));

	//Sphere Properties
	float radius = r;
	int sectorCount = sectorC;
	int stackCount = stackC;
	float sectorStep = 2 * 180 / sectorCount;
	float stackStep = 180 / stackCount;

	float u = 0;
	float v = 0;

	vertices.push_back(Vertex(findCoordinate(sphereNormal, radius), XMFLOAT2(u, v)));

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
		sectorAngle = 0;
	}

	sphereNormal.x = XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(-90));
	sphereNormal.y = XMScalarSin(XMConvertToRadians(-90));
	sphereNormal.z = XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(-90));
	vertices.push_back(Vertex(findCoordinate(sphereNormal, radius), XMFLOAT2(1, 1)));


	//Build north pole indices
	int northPoleIdx = 0;
	int nextIdx = 1;
	for (int i = 0; i < sectorCount; i++) {
	
		if (i == sectorCount - 1) {
			indices.push_back(northPoleIdx);
			indices.push_back(nextIdx);
			indices.push_back(1);
			break;
		}
	
		indices.push_back(northPoleIdx);
		indices.push_back(nextIdx);
		indices.push_back(nextIdx + 1);
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
				indices.push_back(upperStackIdx);
				indices.push_back(lowerStackIdx);
				indices.push_back(upperStackIdx - sectorCount + 1);

				//triangle b
				indices.push_back(upperStackIdx - sectorCount + 1);
				indices.push_back(lowerStackIdx);
				indices.push_back(lowerStackIdx - sectorCount + 1);

				upperStackIdx++;
				lowerStackIdx++;

				break;
			}

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
	}

	// Build south pole indices
	int southPoleIdx = vertices.size() - 1;
	lowerStackIdx = vertices.size() - 1 - sectorCount;

	for (int i = 0; i < sectorCount; i++) {

		if (i == sectorCount - 1) {
			indices.push_back(lowerStackIdx);
			indices.push_back(southPoleIdx);
			indices.push_back(lowerStackIdx - sectorCount + 1);
			break;
		}

		indices.push_back(lowerStackIdx);
		indices.push_back(southPoleIdx);
		indices.push_back(lowerStackIdx + 1);
		lowerStackIdx++;
	}
}

XMFLOAT3 Sphere3d::findCoordinate(XMFLOAT3 unitVector, float radius) {
	return XMFLOAT3(unitVector.x * radius, unitVector.y * radius, unitVector.z * radius);
}

}