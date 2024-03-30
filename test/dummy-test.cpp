#include <unity.h>

void setUp(void)
{
  // set stuff up here
}

void tearDown(void)
{
  // clean stuff up here
}

void should_always_succeed()
{
  TEST_ASSERT_TRUE(true);
}

void should_always_fail()
{
  TEST_ASSERT_TRUE(false);
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();
  RUN_TEST(should_always_succeed);
  RUN_TEST(should_always_fail);
  UNITY_END();
}
