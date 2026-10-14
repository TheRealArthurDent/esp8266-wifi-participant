#include <Arduino.h>

#include "main.hpp"
#include "led.h"
#include "debug.h"
#include "wifi-connection.hpp"
#include "dummy-wifi-dependent.hpp"
#include "metronome.hpp"

const int LOOP_DURATION = 5000;
const int BLINK_INTERVAL = LOOP_DURATION / 2;

Metronome metronome(LOOP_DURATION);

// WifiConnection keeps the pointers it gets in setup(), so the dependents
// must live as long as the program does.
DummyWifiDependent dependent1;
DummyWifiDependent dependent2;

// cppcheck-suppress unusedFunction
void setup()
{

  initDebugging();

  pinMode(led_built_in_ESP, OUTPUT);
  pinMode(led_built_in_Node, OUTPUT);
  digitalWrite(led_built_in_Node, HIGH);
  digitalWrite(led_built_in_ESP, HIGH);

  std::list<WifiDependent *> deps;
  deps.push_back(&dependent1);
  deps.push_back(&dependent2);

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
