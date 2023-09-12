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
}