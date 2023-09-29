#include "maths.h"

namespace tre {

XMMATRIX Maths::createTransformationMatrix(XMFLOAT3 scale, XMFLOAT3 rotation, XMFLOAT3 position) {
	return XMMatrixMultiply(
		XMMatrixScaling(scale.x, scale.y, scale.z),
		XMMatrixMultiply(
			XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)),
			XMMatrixTranslation(
				position.x,
				position.y,
				position.z)
		)
	);
	//return XMMatrixMultiply(
	//	XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)),
	//	XMMatrixMultiply(
	//		XMMatrixScaling(scale.x, scale.y, scale.z),
	//		XMMatrixTranslation(
	//			position.x,
	//			position.y,
	//			position.z)
	//	)
	//);
}

XMVECTOR Maths::getMatrixNormUp(XMMATRIX matrix) {
	XMFLOAT4X4 transformationF;
	XMStoreFloat4x4(&transformationF, matrix);

	return XMVector3Normalize(XMVECTOR{ transformationF._21, transformationF._22, transformationF._23 });
};

XMVECTOR Maths::getMatrixNormRight(XMMATRIX matrix) {
	XMFLOAT4X4 transformationF;
	XMStoreFloat4x4(&transformationF, matrix);

	return XMVector3Normalize(XMVECTOR{ transformationF._11, transformationF._12, transformationF._13 });
};

XMVECTOR Maths::getMatrixNormForward(XMMATRIX matrix) {
	XMFLOAT4X4 transformationF;
	XMStoreFloat4x4(&transformationF, matrix);

	return XMVector3Normalize(XMVECTOR{ transformationF._31, transformationF._32, transformationF._33 });
};

float Maths::distBetweentObjToCam(XMFLOAT3 objPosF, XMVECTOR camPosV) {

	XMVECTOR objPosV{ objPosF.x, objPosF.y, objPosF.z };

	XMVECTOR distFromCamV = XMVector3Length(objPosV - camPosV); // length of vector is replicated in all components 

	XMFLOAT4 distFromCamF;
	XMStoreFloat4(&distFromCamF, distFromCamV);

	return distFromCamF.x;
}

XMFLOAT3 Maths::getRotatePosition(XMFLOAT3 objOrigin, float stackAngle, float sectorAngle, float radius) {
	XMFLOAT3 rotatedPosition;
	rotatedPosition.x = objOrigin.x + radius * XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	rotatedPosition.y = objOrigin.y + radius * XMScalarSin(XMConvertToRadians(stackAngle));
	rotatedPosition.z = objOrigin.z + radius * XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	return rotatedPosition;
}

XMFLOAT3 Maths::XMFLOAT3Addition(XMFLOAT3 a, XMFLOAT3 b) {
	return XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

XMFLOAT3 Maths::XMFLOAT3Minus(XMFLOAT3 a, XMFLOAT3 b) {
	return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
}

XMFLOAT3 Maths::XMFLOAT3ScalarMultiply(XMFLOAT3 a, float x) {
	return XMFLOAT3(a.x * x, a.y * x, a.z * x);
}

float Maths::XMFLOAT3DotProduct(XMFLOAT3 pt1, XMFLOAT3 pt2) {
	XMVECTOR pV1 = XMLoadFloat3(&pt1), pV2 = XMLoadFloat3(&pt2);

	XMFLOAT3 ans;
	XMStoreFloat3(&ans, XMVector3Dot(pV1, pV2));
	return ans.x;
}

void Plane::normalizePlane() {
	float length = sqrtf(this->eqn.x * this->eqn.x + this->eqn.y * this->eqn.y + this->eqn.z * this->eqn.z);

	this->eqn.x /= length;
	this->eqn.y /= length;
	this->eqn.z /= length;
	this->eqn.w /= length;
}

float Plane::getSignedDistanceToPlane(const XMFLOAT3& point) {
	float d = (this->eqn.x * point.x + this->eqn.y * point.y + this->eqn.z * point.z - this->eqn.w) 
		/ sqrtf(this->eqn.x * this->eqn.x + this->eqn.y * this->eqn.y + this->eqn.z * this->eqn.z);
	return d;
}

std::vector<XMVECTOR> Maths::getFrustumCornersWorldSpace(const XMMATRIX& viewProjection) {
	XMMATRIX inverseViewProj = XMMatrixInverse(nullptr, viewProjection);
	
	std::vector<XMVECTOR> corners;

	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 2; z++) {
				XMFLOAT4 pt;
				XMStoreFloat4(&pt, XMVector4Transform(XMVECTOR{ 2.f * x - 1.f, 2.f * y - 1.f, 2.f * z - 1.f, 1.f }, inverseViewProj));
				corners.push_back(XMVECTOR{ pt.x / pt.w, pt.y / pt.w, pt.z / pt.w, 1.f });
			}
		}
	}
	return corners;
}

XMVECTOR Maths::getAverageVector(const std::vector<XMVECTOR>& vectors) {
	
	XMVECTOR resultantV{ .0f, .0f, .0f };
	
	for (const XMVECTOR& vector : vectors) {
		XMFLOAT4 vectorF;
		XMStoreFloat4(&vectorF, vector);

		resultantV += XMVECTOR{ vectorF.x, vectorF.y, vectorF.z };
	}

	return resultantV / vectors.size();
}

XMMATRIX Maths::createOrthoMatrixFromFrustumCorners(float zMult, const std::vector<XMVECTOR>& corners, const XMMATRIX& viewMatrix) {
	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();

	for (const XMVECTOR& c : corners) {
		XMFLOAT3 cF;
		XMStoreFloat3(&cF, c);
		
		XMFLOAT4 viewspaceCoor;
		XMStoreFloat4(&viewspaceCoor, XMVector4Transform(XMVECTOR{ cF.x, cF.y, cF.z, 1.f }, viewMatrix)); // check if need to transpose viewMatrix 

		minX = std::min(minX, viewspaceCoor.x);
		maxX = std::max(maxX, viewspaceCoor.x);
		minY = std::min(minY, viewspaceCoor.y);
		maxY = std::max(maxY, viewspaceCoor.y);
		minZ = std::min(minZ, viewspaceCoor.z);
		maxZ = std::max(maxZ, viewspaceCoor.z);
	}

	if (minZ < 0) {
		minZ *= zMult;
	} else {
		minZ /= zMult;
	}
	
	if (maxZ < 0) {
		maxZ /= zMult;
	} else {
		maxZ *= zMult;
	}

	return XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
}

}