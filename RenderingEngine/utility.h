#pragma once

#include <DirectXMath.h>
#include <microprofile.h>
#define PROFILE_GPU_SCOPED(NAME) MICROPROFILE_SCOPEGPUI(NAME, tre::Utility::getRandomInt(INT_MAX))

#include <string>
#include <assert.h>
#include <iostream>
#include <codecvt>
#include <locale>
#include <regex>
#include <random>

using namespace DirectX;

namespace tre {
class Utility {

public:
	static std::string getBasePathStr();
	static std::string getDirPathStr(std::string filepath);
	static std::wstring getBasePathWstr();
	static std::wstring convertToWstr(std::string str);
	static std::string convertToStr(std::wstring wstr);
	static std::string uriDecode(std::string str);
	static int getRandomInt(int maxValue); // inclusive of max
	static int getRandomIntRange(int minValue, int maxValue); // inclusive min and max
	static float getRandomFloat(float maxValue);
	static float getRandomFloatRange(float minValue, float maxValue);
};
}