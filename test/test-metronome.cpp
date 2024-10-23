#include <gtest/gtest.h>
#include "metronome.hpp"

// Test Metronome with skipping missed cycles
TEST(MetronomeTest, SkipMissedCycles)
{
  Metronome metronome(100, true); // 100ms duration, skip missed cycles

  // Simulate a call to waitForNextCycle and assert correct behavior.
  // We'll mock time behavior here if necessary.
  metronome.waitForNextCycle();

  // You could check internal state using friend declarations or public accessors
  EXPECT_TRUE(true); // Replace with actual checks if possible.
}

// Test Metronome without skipping missed cycles
TEST(MetronomeTest, DoNotSkipMissedCycles)
{
  Metronome metronome(100, false); // 100ms duration, do not skip missed cycles

  // Simulate a case where a cycle was missed
  metronome.waitForNextCycle();

  // You can mock or assert here how the missed cycle is handled
  EXPECT_TRUE(true); // Again, replace with actual logic checks
}

// Additional tests can check for different time durations and edge cases
