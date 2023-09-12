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
};
}