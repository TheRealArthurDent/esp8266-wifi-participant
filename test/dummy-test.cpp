#include <gtest/gtest.h>

// Simple test case
TEST(SampleTest, BasicAssertions)
{
  EXPECT_EQ(1 + 1, 2);
  EXPECT_TRUE(true);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
