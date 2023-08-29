#include "utility.h"

namespace tre {

Utility::Utility() {
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

int Utility::genRandomInt(int maxValue) {
	return rand() % maxValue;
}

}