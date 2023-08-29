#pragma once

#include <string>
#include <iostream>
#include <codecvt>
#include <locale>
#include <assert.h>

using namespace std;

namespace tre {
class Utility {

public:
	string basePathStr;
	wstring basePathWstr;

	Utility();

	wstring convertToWstr(string str);
	string convertToStr(wstring wstr);
	static int getRandomInt(int maxValue); // inclusive of max
	static int getRandomIntRange(int minValue, int maxValue); // inclusive min and max
	static float getRandomFloat(float maxValue);
	static float getRandomFloatRange(float minValue, float maxValue);
};
}