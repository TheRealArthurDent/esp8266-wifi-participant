#include <ArduinoFake.h>
#include <unity.h>

#include "fakeit-unity.hpp"

#include "debug.h"

using fakeit::Verify;
using fakeit::VerifyNoOtherInvocations;
using fakeit::When;

void setUp()
{
  ArduinoFakeReset();
  When(OverloadedMethod(ArduinoFake(Serial), begin, void(unsigned long))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *))).AlwaysReturn(0);
  When(OverloadedMethod(ArduinoFake(Serial), println, size_t())).AlwaysReturn(0);
}

void tearDown()
{
}

#ifdef DEBUG
void opensTheSerialPortAtTheMonitorSpeed()
{
  initDebugging();

  Verify(OverloadedMethod(ArduinoFake(Serial), begin, void(unsigned long)).Using(MONITOR_SPEED)).Once();
}

void logsThatDebuggingIsInitialized()
{
  initDebugging();

  Verify(OverloadedMethod(ArduinoFake(Serial), begin, void(unsigned long)),
         OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *)).Using(fakeit::_),
         OverloadedMethod(ArduinoFake(Serial), println, size_t()));
}
#else
void leavesTheSerialPortAloneInReleaseBuilds()
{
  initDebugging();

  VerifyNoOtherInvocations(ArduinoFake(Serial));
}
#endif

auto main() -> int
{
  installFakeitUnityAdapter();
  UNITY_BEGIN();
#ifdef DEBUG
  RUN_TEST(opensTheSerialPortAtTheMonitorSpeed);
  RUN_TEST(logsThatDebuggingIsInitialized);
#else
  RUN_TEST(leavesTheSerialPortAloneInReleaseBuilds);
#endif
  return UNITY_END();
}
