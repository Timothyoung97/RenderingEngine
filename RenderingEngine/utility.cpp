#include <assert.h>
#include <iostream>
#include <codecvt>
#include <locale>

#include "utility.h"

namespace tre {

Utility::Utility() {

	srand((uint32_t) time(NULL));

	std::string filepath = __FILE__;
	size_t lastSlash = filepath.find_last_of("\\/");
	basePathStr = filepath.substr(0, lastSlash + 1);
	basePathWstr = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(filepath.substr(0, lastSlash + 1));
}

std::wstring Utility::convertToWstr(std::string str) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
}

std::string Utility::convertToStr(std::wstring wstr) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
}

int Utility::getRandomInt(int maxValue) {
	return getRandomIntRange(0, maxValue);
}

int Utility::getRandomIntRange(int minValue, int maxValue) { 
	assert(maxValue > minValue);
	return rand() % (maxValue - minValue + 1) + minValue;
}

float Utility::getRandomFloat(float maxValue) {
	return getRandomFloatRange(0, maxValue);
}

float Utility::getRandomFloatRange(float minValue, float maxValue) {
	assert(maxValue > minValue);
	return minValue + static_cast<float>(rand()) * static_cast<float>(maxValue - minValue) / RAND_MAX;;
}

float Utility::distBetweentObjToCam(XMFLOAT3 objPosF, XMVECTOR camPosV) {

	XMVECTOR objPosV{ objPosF.x, objPosF.y, objPosF.z };

	XMVECTOR distFromCamV = XMVector3Length(objPosV - camPosV); // length of vector is replicated in all components 

	XMFLOAT4 distFromCamF;
	XMStoreFloat4(&distFromCamF, distFromCamV);

	return distFromCamF.x;
}

XMFLOAT3 Utility::getRotatePosition(XMFLOAT3 objOrigin, float stackAngle, float sectorAngle, float radius) {
	XMFLOAT3 rotatedPosition;
	rotatedPosition.x = objOrigin.x + radius * XMScalarCos(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	rotatedPosition.y = objOrigin.y + radius * XMScalarSin(XMConvertToRadians(stackAngle));
	rotatedPosition.z = objOrigin.z + radius * XMScalarSin(XMConvertToRadians(sectorAngle)) * XMScalarCos(XMConvertToRadians(stackAngle));
	return rotatedPosition;
}

XMFLOAT3 Utility::XMFLOAT3Addition(XMFLOAT3 a, XMFLOAT3 b) {
	return XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

XMFLOAT3 Utility::XMFLOAT3Minus(XMFLOAT3 a, XMFLOAT3 b) {
	return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
}

XMFLOAT3 Utility::XMFLOAT3ScalarMultiply(XMFLOAT3 a, float x) {
	return XMFLOAT3(a.x * x, a.y * x, a.z * x);
}

float Utility::XMFLOAT3DotProduct(XMFLOAT3 pt1, XMFLOAT3 pt2) {
	XMVECTOR pV1 = XMLoadFloat3(&pt1), pV2 = XMLoadFloat3(&pt2);

	XMFLOAT3 ans;
	XMStoreFloat3(&ans, XMVector3Dot(pV1, pV2));
	return ans.x;
}


}