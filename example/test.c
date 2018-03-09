#include "test.h"

#include <limits.h>

TEST_PACKAGE(foo)

TEST(case1) {

	int a = 2;
    int b = 2;
    TEST_ASSERT(a == b);

    TEST_LOG("LOG TRUE");
    TEST_ASSERT(1);
}

TEST(case2) {
	TEST_ASSERT(0);
}

TEST(case3) {
	TEST_ASSERT(1);
}

START_TEST();
