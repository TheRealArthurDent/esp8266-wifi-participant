#include "debug.h"

// cppcheck-suppress unusedFunction
void init_debugging()
{
#ifdef DEBUG
  Serial.begin(MONITOR_SPEED);
#endif
  DEBUG_PRINTLN("Debugging initialized.");
  DEBUG_PRINTLN();
}
