#pragma once

#include <DirectXMath.h>

#include <string>

using namespace DirectX;

namespace tre {
class Utility {

public:
	std::string basePathStr;
	std::wstring basePathWstr;

	Utility();

	std::wstring convertToWstr(std::string str);
	std::string convertToStr(std::wstring wstr);
	static int getRandomInt(int maxValue); // inclusive of max
	static int getRandomIntRange(int minValue, int maxValue); // inclusive min and max
	static float getRandomFloat(float maxValue);
	static float getRandomFloatRange(float minValue, float maxValue);
	static float distBetweentObjToCam(XMFLOAT3 objPosF, XMVECTOR camPosV);
	static XMFLOAT3 getRotatePosition(XMFLOAT3 objOrigin, float stackAngle, float sectorAngle, float radius);
	static float XMFLOAT3DotProduct(XMFLOAT3 pt1, XMFLOAT3 pt2);
	static XMFLOAT3 XMFLOAT3Addition(XMFLOAT3 a, XMFLOAT3 b);
	static XMFLOAT3 XMFLOAT3Minus(XMFLOAT3 a, XMFLOAT3 b);
	static XMFLOAT3 XMFLOAT3ScalarMultiply(XMFLOAT3 a, float x);
};
}