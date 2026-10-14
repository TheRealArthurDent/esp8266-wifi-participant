#include <Arduino.h>

#include "led-blinker.hpp"
#include "led.h"
#include "debug.h"

// cppcheck-suppress unusedFunction
void led_blinker::blink()
{
  digitalWrite(LED_BUILT_IN_NODE, LOW);
  delay(blink_interval);
  digitalWrite(LED_BUILT_IN_NODE, HIGH);

  beat.wait_for_next_cycle();

  DEBUG_PRINT("Loop ended at: ");
  DEBUG_PRINTLN(millis());
}
