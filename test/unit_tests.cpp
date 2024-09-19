#include <gtest/gtest.h>


TEST(FirstTest, TrivialEquality) {
	EXPECT_EQ(42, 42);
}

TEST(FirstTest, MoreEqualityTests) {
	EXPECT_FLOAT_EQ(23.23F, 23.23F);
}
