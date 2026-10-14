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
void logsAnEstablishedConnection()
{
  DummyWifiDependent dependent;

  dependent.onWifiConnectionEstablished();

  Verify(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *))
             .Matching([](const char *line)
                       { return std::string(line) == "DummyWifiDependent connected!"; }))
      .Once();
  VerifyNoOtherInvocations(ArduinoFake(Serial));
}

void logsALostConnection()
{
  DummyWifiDependent dependent;

  dependent.onWifiConnectionLost();

  Verify(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *))
             .Matching([](const char *line)
                       { return std::string(line) == "DummyWifiDependent disconnected!"; }))
      .Once();
  VerifyNoOtherInvocations(ArduinoFake(Serial));
}
#else
void staysSilentInReleaseBuilds()
{
  DummyWifiDependent dependent;

  dependent.onWifiConnectionEstablished();
  dependent.onWifiConnectionLost();

  VerifyNoOtherInvocations(ArduinoFake(Serial));
}
#endif

auto main() -> int
{
  installFakeitUnityAdapter();
  UNITY_BEGIN();
#ifdef DEBUG
  RUN_TEST(logsAnEstablishedConnection);
  RUN_TEST(logsALostConnection);
#else
  RUN_TEST(staysSilentInReleaseBuilds);
#endif
  return UNITY_END();
}
