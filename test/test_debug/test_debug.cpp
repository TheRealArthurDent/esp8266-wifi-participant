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
void opens_the_serial_port_at_the_monitor_speed()
{
  init_debugging();

  Verify(OverloadedMethod(ArduinoFake(Serial), begin, void(unsigned long)).Using(MONITOR_SPEED)).Once();
}

void logs_that_debugging_is_initialized()
{
  init_debugging();

  Verify(OverloadedMethod(ArduinoFake(Serial), begin, void(unsigned long)),
         OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *)).Using(fakeit::_),
         OverloadedMethod(ArduinoFake(Serial), println, size_t()));
}
#else
void leaves_the_serial_port_alone_in_release_builds()
{
  init_debugging();

  VerifyNoOtherInvocations(ArduinoFake(Serial));
}
#endif

auto main() -> int
{
  install_fakeit_unity_adapter();
  UNITY_BEGIN();
#ifdef DEBUG
  RUN_TEST(opens_the_serial_port_at_the_monitor_speed);
  RUN_TEST(logs_that_debugging_is_initialized);
#else
  RUN_TEST(leaves_the_serial_port_alone_in_release_builds);
#endif
  return UNITY_END();
}
