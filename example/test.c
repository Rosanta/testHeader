#include "test.h"

#include <limits.h>

#if ULONG_MAX == UINT64_MAX
#define RES  "64BTIS"
#else 
#define RES "32BITS"
#endif

TEST_PACKAGE(foo)

TEST(bar) {
    TEST_ASSERT(1);

}

#if 1
TEST(chenfei) {

    int a ,b;
    a = 1;
    b = 2;
    TEST_ASSERT(a == b);

    TEST_LOG("LOG TRUE");
    TEST_ASSERT(1);
}
#endif

START_TEST()
