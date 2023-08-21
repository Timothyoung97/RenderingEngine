#pragma once

#define CHECK_DX11_ERROR(dx11Func, ...) \
{ \
	HRESULT hresult; \
	if (!SUCCEEDED(hresult = dx11Func(__VA_ARGS__))) { \
		std::printf("Assertion failed: %d at %s:%d\n", hresult, __FILE__, __LINE__);	\
		assert(false); \
	} \
}

