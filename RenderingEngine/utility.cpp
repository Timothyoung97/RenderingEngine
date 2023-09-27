#include <assert.h>
#include <iostream>
#include <codecvt>
#include <locale>
#include <regex>

#include "utility.h"

namespace tre {

std::wstring Utility::convertToWstr(std::string str) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
}

std::string Utility::convertToStr(std::wstring wstr) {
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
}

std::string Utility::uriDecode(std::string str) {
	std::regex toReplace("%20");
	std::string res = std::regex_replace(str, toReplace, " ");
	return res;
}

std::string Utility::getBasePathStr() {
	std::string filepath = __FILE__;
	size_t lastSlash = filepath.find_last_of("\\/");
	return filepath.substr(0, lastSlash + 1);
}

std::string Utility::getDirPathStr(std::string filepath) {
	size_t lastSlash = filepath.find_last_of("\\/");
	return filepath.substr(0, lastSlash + 1);
}

std::wstring Utility::getBasePathWstr() {
	std::string filepath = __FILE__;
	size_t lastSlash = filepath.find_last_of("\\/");
	return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(filepath.substr(0, lastSlash + 1));
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