#include <Arduino.h>

#include "led.h"
#include "debug.h"
#include "wifi-connection.hpp"
#include "dummy-wifi-dependent.hpp"
#include "led-blinker.hpp"

const int loop_duration = 5000;

led_blinker blinker(loop_duration);

// cppcheck-suppress unusedFunction
void setup()
{

  init_debugging();

  pinMode(LED_BUILT_IN_ESP, OUTPUT);
  pinMode(LED_BUILT_IN_NODE, OUTPUT);
  digitalWrite(LED_BUILT_IN_NODE, HIGH);
  digitalWrite(LED_BUILT_IN_ESP, HIGH);

  // wifi_connection keeps these pointers, so the dependents must outlive
  // setup(): static locals live as long as the program does.
  static dummy_wifi_dependent dependent1;
  static dummy_wifi_dependent dependent2;

  std::list<wifi_dependent *> deps;
  deps.push_back(&dependent1);
  deps.push_back(&dependent2);

  wifi_connection::get_instance().init(deps);
}

// cppcheck-suppress unusedFunction
void loop()
{
  blinker.blink();
}
