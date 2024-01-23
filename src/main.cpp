
#include <Arduino.h>
#include "main.hpp"
#include "led.h"
#include "debug.h"
#include "wifi-connection.hpp"
#include "dummy-wifi-dependent.hpp"
#include "metronome.hpp"

const int LOOP_DURATION = 5000;
const int BLINK_INTERVAL = LOOP_DURATION / 2;

// This "class" is our helper to get us from non-OO to an OO context.
// Since we're not within a class yet, we have to declare it globally.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
Metronome metronome(LOOP_DURATION);

// cppcheck-suppress unusedFunction
void setup()
{

  initDebugging();

  pinMode(led_built_in_ESP, OUTPUT);
  pinMode(led_built_in_Node, OUTPUT);
  digitalWrite(led_built_in_Node, HIGH);
  digitalWrite(led_built_in_ESP, HIGH);

  DummyWifiDependent dependent1;
  DummyWifiDependent dependent2;

  std::list<WifiDependent *> deps{&dependent1, &dependent2};

  WifiConnection::getInstance().init(deps);
}

// cppcheck-suppress unusedFunction
void loop()
{
  blinkLed();
}

void blinkLed()
{
  digitalWrite(led_built_in_Node, LOW);
  delay(BLINK_INTERVAL);
  digitalWrite(led_built_in_Node, HIGH);

  metronome.waitForNextCycle();

  DEBUG_PRINT("Loop ended at: ");
  DEBUG_PRINTLN(millis());
}
