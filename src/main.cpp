#include <Arduino.h>

#include "led.h"
#include "debug.h"
#include "wifi-connection.hpp"
#include "dummy-wifi-dependent.hpp"
#include "led-blinker.hpp"

const int LOOP_DURATION = 5000;

LedBlinker ledBlinker(LOOP_DURATION);

// cppcheck-suppress unusedFunction
void setup()
{

  initDebugging();

  pinMode(led_built_in_ESP, OUTPUT);
  pinMode(led_built_in_Node, OUTPUT);
  digitalWrite(led_built_in_Node, HIGH);
  digitalWrite(led_built_in_ESP, HIGH);

  // WifiConnection keeps these pointers, so the dependents must outlive
  // setup(): static locals live as long as the program does.
  static DummyWifiDependent dependent1;
  static DummyWifiDependent dependent2;

  std::list<WifiDependent *> deps;
  deps.push_back(&dependent1);
  deps.push_back(&dependent2);

  WifiConnection::getInstance().init(deps);
}

// cppcheck-suppress unusedFunction
void loop()
{
  ledBlinker.blink();
}
