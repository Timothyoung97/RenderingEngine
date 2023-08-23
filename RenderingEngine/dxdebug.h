#pragma once
#include <string>
#include <cassert>

#define CHECK_DX_ERROR(dx11Func) \
{ \
	HRESULT hresult; \
	if (!SUCCEEDED(hresult = dx11Func)) { \
		std::printf("Assertion failed: %d at %s:%d\n", hresult, __FILE__, __LINE__);	\
		assert(false); \
	} \
}

