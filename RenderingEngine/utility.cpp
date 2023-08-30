#include "utility.h"

namespace tre {

Utility::Utility() {

	srand(time(NULL));

	string filepath = __FILE__;
	size_t lastSlash = filepath.find_last_of("\\/");
	basePathStr = filepath.substr(0, lastSlash + 1);
	basePathWstr = wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(filepath.substr(0, lastSlash + 1));
}

wstring Utility::convertToWstr(string str) {
	return wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(str);
}

string Utility::convertToStr(wstring wstr) {
	return wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
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