#pragma once

#include "metronome.hpp"

/**
 * \brief Blinks the node LED once per cycle.
 *
 * Shows that the firmware is alive: every call to blink() switches the node
 * LED on for half a cycle, then waits for the next cycle to start.
 */
class led_blinker
{
public:
  /**
   * \brief Constructor.
   *
   * \param cycle_duration The duration of one blink cycle in milliseconds.
   */
  explicit led_blinker(int cycle_duration)
      : blink_interval(cycle_duration / 2), beat(cycle_duration) {}

  /**
   * \brief Blinks the node LED once and blocks until the next cycle starts.
   */
  void blink();

private:
  int blink_interval;
  metronome beat;
};
