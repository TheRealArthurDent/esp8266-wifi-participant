#include <Arduino.h>
#include "metronome.hpp"

// cppcheck-suppress unusedFunction
void Metronome::waitForNextCycle()
{
  const Millis now = millis();
  const auto duration = static_cast<Millis>(cycle_duration);

  if (skip_missed_cycles || !started)
  {
    const Millis wait = duration - (now % duration);
    current_cycle_start = now + wait;
    started = true;
    delay(wait);
    return;
  }

  // Unsigned arithmetic keeps this right when millis() wraps around.
  const Millis elapsed = now - current_cycle_start;
  current_cycle_start += duration;
  delay(elapsed >= duration ? 0 : duration - elapsed);
}
