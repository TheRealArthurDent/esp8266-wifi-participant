#include <Arduino.h>

#include "led-blinker.hpp"
#include "led.h"
#include "debug.h"

// cppcheck-suppress unusedFunction
void LedBlinker::blink()
{
  digitalWrite(LED_BUILT_IN_NODE, LOW);
  delay(blink_interval);
  digitalWrite(LED_BUILT_IN_NODE, HIGH);

  metronome.waitForNextCycle();

  DEBUG_PRINT("Loop ended at: ");
  DEBUG_PRINTLN(millis());
}
