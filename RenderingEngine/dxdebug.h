#pragma once

#include <spdlog/spdlog.h>

#define CHECK_DX_ERROR(dx11Func) \
{ \
	HRESULT hresult; \
	if (!SUCCEEDED(hresult = dx11Func)) { \
		spdlog::error("Assertion failed: {} at {}:{}\n", hresult, __FILE__, __LINE__);	\
		assert(false); \
	} \
}

