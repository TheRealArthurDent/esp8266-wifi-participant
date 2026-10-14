#include <ArduinoFake.h>
#include <unity.h>

#include "fakeit-unity.hpp"

#include "led-blinker.hpp"
#include "led.h"

using fakeit::Verify;
using fakeit::When;

namespace
{
const int cycle_duration = 1000;
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

void switches_the_node_led_on_for_half_a_cycle_then_waits_for_the_next_one()
{
  led_blinker blinker(cycle_duration);

  blinker.blink();

  // millis() is 1500 after the LED went off, so the next cycle is 500 ms away.
  Verify(Method(ArduinoFake(), digitalWrite).Using(LED_BUILT_IN_NODE, LOW),
         Method(ArduinoFake(), delay).Using(cycle_duration / 2),
         Method(ArduinoFake(), digitalWrite).Using(LED_BUILT_IN_NODE, HIGH),
         Method(ArduinoFake(), delay).Using(500));
}

void leaves_the_esp_led_alone()
{
  led_blinker blinker(cycle_duration);

  blinker.blink();

  Verify(Method(ArduinoFake(), digitalWrite).Using(LED_BUILT_IN_ESP, fakeit::_)).Never();
}

void logs_the_end_of_the_cycle_in_debug_builds()
{
  led_blinker blinker(cycle_duration);

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
  install_fakeit_unity_adapter();
  UNITY_BEGIN();
  RUN_TEST(switches_the_node_led_on_for_half_a_cycle_then_waits_for_the_next_one);
  RUN_TEST(leaves_the_esp_led_alone);
  RUN_TEST(logs_the_end_of_the_cycle_in_debug_builds);
  return UNITY_END();
}
