#include <Arduino.h>
#include "metronome.hpp"

// cppcheck-suppress unusedFunction
void Metronome::waitForNextCycle()
{
  const Millis now = millis();
  const auto duration = static_cast<Millis>(cycleDuration);

  if (skipMissedCycles || !started)
  {
    const Millis wait = duration - (now % duration);
    currentCycleStart = now + wait;
    started = true;
    delay(wait);
    return;
  }

  // Unsigned arithmetic keeps this right when millis() wraps around.
  const Millis elapsed = now - currentCycleStart;
  currentCycleStart += duration;
  delay(elapsed >= duration ? 0 : duration - elapsed);
}
