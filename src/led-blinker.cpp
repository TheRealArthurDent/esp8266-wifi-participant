#include <Arduino.h>

#include "led-blinker.hpp"
#include "led.h"
#include "debug.h"

// cppcheck-suppress unusedFunction
void LedBlinker::blink()
{
  digitalWrite(led_built_in_Node, LOW);
  delay(blinkInterval);
  digitalWrite(led_built_in_Node, HIGH);

  metronome.waitForNextCycle();

  DEBUG_PRINT("Loop ended at: ");
  DEBUG_PRINTLN(millis());
}
