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
}