#include "Arduino.h"

#include "arduino-callable.hpp"
#include "app.hpp"

ArduinoCallable *delegate; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// cppcheck-suppress unusedFunction
void setup()
{
  delegate = new App();
  delegate->setup();
}

// cppcheck-suppress unusedFunction
void loop()
{
  delegate->loop();
}
