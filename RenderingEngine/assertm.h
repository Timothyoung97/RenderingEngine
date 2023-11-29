
#define CHECK_ERROR(expectedCondition, expectMsg) \
{ \
	if (!(expectedCondition)) { \
		printf("Assertion failed at %s:%d\n%s", __FILE__, __LINE__, expectMsg);	\
		assert(false); \
	} \
}