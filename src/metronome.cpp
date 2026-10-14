#include <Arduino.h>
#include "metronome.hpp"

// cppcheck-suppress unusedFunction
void metronome::wait_for_next_cycle()
{
  const milliseconds now = millis();
  const auto duration = static_cast<milliseconds>(cycle_duration);

  if (skip_missed_cycles || !started)
  {
    const milliseconds wait = duration - (now % duration);
    current_cycle_start = now + wait;
    started = true;
    delay(wait);
    return;
  }

  // Unsigned arithmetic keeps this right when millis() wraps around.
  const milliseconds elapsed = now - current_cycle_start;
  current_cycle_start += duration;
  delay(elapsed >= duration ? 0 : duration - elapsed);
}
