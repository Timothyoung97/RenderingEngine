#pragma once

#include <string>
#include <iostream>
#include <codecvt>
#include <locale>

using namespace std;

namespace tre {
class Utility {

public:
	string basePathStr;
	wstring basePathWstr;

	Utility();

	wstring convertToWstr(string str);
	string convertToStr(wstring wstr);
};
}