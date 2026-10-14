#include <ArduinoFake.h>
#include <unity.h>

#include "fakeit-unity.hpp"

#include "dummy-wifi-dependent.hpp"

using fakeit::Verify;
using fakeit::VerifyNoOtherInvocations;
using fakeit::When;

void setUp()
{
  ArduinoFakeReset();
  When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *))).AlwaysReturn(0);
}

void tearDown()
{
}

#ifdef DEBUG
void logs_an_established_connection()
{
  dummy_wifi_dependent dependent;

  dependent.on_wifi_connection_established();

  Verify(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *))
             .Matching([](const char *line)
                       { return std::string(line) == "dummy_wifi_dependent connected!"; }))
      .Once();
  VerifyNoOtherInvocations(ArduinoFake(Serial));
}

void logs_a_lost_connection()
{
  dummy_wifi_dependent dependent;

  dependent.on_wifi_connection_lost();

  Verify(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *))
             .Matching([](const char *line)
                       { return std::string(line) == "dummy_wifi_dependent disconnected!"; }))
      .Once();
  VerifyNoOtherInvocations(ArduinoFake(Serial));
}
#else
void stays_silent_in_release_builds()
{
  dummy_wifi_dependent dependent;

  dependent.on_wifi_connection_established();
  dependent.on_wifi_connection_lost();

  VerifyNoOtherInvocations(ArduinoFake(Serial));
}
#endif

auto main() -> int
{
  install_fakeit_unity_adapter();
  UNITY_BEGIN();
#ifdef DEBUG
  RUN_TEST(logs_an_established_connection);
  RUN_TEST(logs_a_lost_connection);
#else
  RUN_TEST(stays_silent_in_release_builds);
#endif
  return UNITY_END();
}
