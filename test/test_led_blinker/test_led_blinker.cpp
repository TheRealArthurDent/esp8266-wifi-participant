#include <ArduinoFake.h>
#include <unity.h>

#include "fakeit-unity.hpp"

#include "led-blinker.hpp"
#include "led.h"

using fakeit::Verify;
using fakeit::When;

namespace
{
const int CYCLE_DURATION = 1000;
} // namespace

void setUp()
{
  ArduinoFakeReset();
  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
  When(Method(ArduinoFake(), delay)).AlwaysReturn();
  When(Method(ArduinoFake(), millis)).AlwaysReturn(1500);
  // The DEBUG build logs the end of each cycle.
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char *))).AlwaysReturn(0);
  When(OverloadedMethod(ArduinoFake(Serial), println, size_t(unsigned long, int))).AlwaysReturn(0);
}

void tearDown()
{
}

void switchesTheNodeLedOnForHalfACycleThenWaitsForTheNextOne()
{
  LedBlinker blinker(CYCLE_DURATION);

  blinker.blink();

  // millis() is 1500 after the LED went off, so the next cycle is 500 ms away.
  Verify(Method(ArduinoFake(), digitalWrite).Using(LED_BUILT_IN_NODE, LOW),
         Method(ArduinoFake(), delay).Using(CYCLE_DURATION / 2),
         Method(ArduinoFake(), digitalWrite).Using(LED_BUILT_IN_NODE, HIGH),
         Method(ArduinoFake(), delay).Using(500));
}

void leavesTheEspLedAlone()
{
  LedBlinker blinker(CYCLE_DURATION);

  blinker.blink();

  Verify(Method(ArduinoFake(), digitalWrite).Using(LED_BUILT_IN_ESP, fakeit::_)).Never();
}

void logsTheEndOfTheCycleInDebugBuilds()
{
  LedBlinker blinker(CYCLE_DURATION);

  blinker.blink();

#ifdef DEBUG
  Verify(OverloadedMethod(ArduinoFake(Serial), println, size_t(unsigned long, int)).Using(1500UL, DEC)).Once();
#else
  using fakeit::VerifyNoOtherInvocations;
  VerifyNoOtherInvocations(ArduinoFake(Serial));
#endif
}

auto main() -> int
{
  installFakeitUnityAdapter();
  UNITY_BEGIN();
  RUN_TEST(switchesTheNodeLedOnForHalfACycleThenWaitsForTheNextOne);
  RUN_TEST(leavesTheEspLedAlone);
  RUN_TEST(logsTheEndOfTheCycleInDebugBuilds);
  return UNITY_END();
}
