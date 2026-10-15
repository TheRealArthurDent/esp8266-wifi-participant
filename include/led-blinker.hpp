#pragma once

#include "metronome.hpp"

/**
 * \brief Blinks the node LED once per cycle.
 *
 * Shows that the firmware is alive: every call to blink() switches the node
 * LED on for half a cycle, then waits for the next cycle to start.
 */
class LedBlinker
{
public:
  /**
   * \brief Constructor.
   *
   * \param cycle_duration The duration of one blink cycle in milliseconds.
   */
  explicit LedBlinker(int cycle_duration)
      : blink_interval(cycle_duration / 2), metronome(cycle_duration) {}

  /**
   * \brief Blinks the node LED once and blocks until the next cycle starts.
   */
  void blink();

private:
  int blink_interval;
  Metronome metronome;
};
