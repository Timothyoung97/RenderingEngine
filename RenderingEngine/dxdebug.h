#pragma once

#include <stdio.h>
#include <cassert>
#include <dxgitype.h>

#define CHECK_DX_ERROR(dx11Func) \
{ \
	HRESULT hresult; \
	if (!SUCCEEDED(hresult = dx11Func)) { \
		printf("Assertion failed: %#010x at %s:%d\n", hresult, __FILE__, __LINE__);	\
		assert(false); \
	} \
}

