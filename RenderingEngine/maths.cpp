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
	XMVECTOR planeNormal{ this->eqn.x, this->eqn.y, this->eqn.z };
	XMVECTOR sphereCenter{ point.x, point.y, point.z };

	XMVECTOR dotProduct = XMVector3Dot(planeNormal, sphereCenter);

	XMFLOAT3 dotProductF;
	XMStoreFloat3(&dotProductF, dotProduct);

	return dotProductF.x - this->eqn.w;
};


}