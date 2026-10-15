#pragma once

/**
 * \brief The metronome.
 *
 * A helper class that lets you manage (somewhat) constant durations
 * of your loop() function. Define a cycle_duration and call waitForNextCycle()
 * at the end of your loop.
 * Easy, isn't it? :)
 */
class Metronome
{
public:
  /**
   * \brief Constructor.
   *
   * \param cycle_duration The duration of one cycle of the loop.
   * \param skip_missed_cycles
   * \parblock
   * In case a cycle has been missed - due to stuff happening within the loop
   * taking more time than the defined cycle duration - this bool defines
   * whether the missed cycle should be skipped or not.
   * In this case \c true will make metronome's \c waitForNextCycle just wait
   * until the next regular cycle, thereby skipping at least one cycle,
   * whereas \c false will make metronome's \c waitForNextCycle return
   * immediately and thereby trigger the next iteration of the loop in order
   * to make up the delay.
   * \warning Using \c skip_missed_cycles = \c false in conjunction with a loop
   * that usually takes more time than \c cycle_duration will lead to a
   * growing delay that can never be made up.
   * \endparblock
   */
  Metronome(int cycle_duration, bool skip_missed_cycles = true)
      : cycle_duration(cycle_duration), skip_missed_cycles(skip_missed_cycles) {}

  /**
   * \brief Blocks until the next cycle starts.
   *
   * Might return immediately if the Metronome has been configured NOT to skip
   * missed cycles ( \c skip_missed_cycles = \c false ).
   * See Metronome(int, bool).
   */
  auto waitForNextCycle() -> void;

  Metronome() = delete;
  Metronome(Metronome const &) = delete;
  Metronome(Metronome &&) = delete;
  auto operator=(Metronome const &) -> Metronome & = delete;
  auto operator=(Metronome &&) -> Metronome & = delete;
  ~Metronome() = default;

private:
  /// The type \c millis() returns.
  using Millis = unsigned long; // NOLINT(google-runtime-int)

  int cycle_duration;
  bool skip_missed_cycles;
  bool started = false;
  Millis current_cycle_start = 0;
};
