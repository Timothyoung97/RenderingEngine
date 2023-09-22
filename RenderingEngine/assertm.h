#include <spdlog/spdlog.h>

#define CHECK_ERROR(expectedCondition, expectMsg) \
{ \
	if (!(expectedCondition)) { \
		spdlog::error("Assertion failed: {} at {}:{}\n{}", expectedCondition, __FILE__, __LINE__, expectMsg);	\
		assert(false); \
	} \
}