#pragma once

/**
 * \brief The metronome.
 *
 * A helper class that let's you manage (somewhat) constant durations
 * of your loop() function. Define a cycle_duration and call wait_for_next_cycle()
 * at the end of your loop.
 * Easy, isn't it? :)
 */
class metronome
{
public:
  /**
   * \brief Constructor.
   *     *
   * \param cycle_duration The duration of one cycle of the loop.
   * \param skip_missed_cycles
   * \parblock
   * In case a cycle has been missed - due to stuff happening within the loop
   * taking more time than the defined cycle duration - this bool defines
   * whether the missed cycle should be skipped or not.
   * In this case \c true will make metronome's \c wait_for_next_cycle just wait
   * until the next regular cycle, thereby skipping at least one cycle,
   * whereas \c false will make metronome's \c wait_for_next_cycle return
   * immediately and thereby trigger the next iteration of the loop in order
   * to make up the delay.
   * \warning Using \c skip_missed_cycles = \c false in conjunction with a loop
   * that usually takes more time than \c cycle_duration will lead to a
   * growing delay that can never be made up.
   * \endparblock
   */
  metronome(int cycle_duration, bool skip_missed_cycles = true)
      : cycle_duration(cycle_duration), skip_missed_cycles(skip_missed_cycles) {}

  /**
   * Blocks until the next cycle starts.
   * Might return immediately if the metronome has been configured NOT to skip
   * missed cycles ( \c skip_missed_cycles = \c false ).
   * See metronome(int, bool).
   */
  auto wait_for_next_cycle() -> void;

  metronome() = delete;
  metronome(metronome const &) = delete;
  metronome(metronome &&) = delete;
  auto operator=(metronome const &) -> metronome & = delete;
  auto operator=(metronome &&) -> metronome & = delete;
  ~metronome() = default;

private:
  /// The type \c millis() returns.
  using milliseconds = unsigned long; // NOLINT(google-runtime-int)

  int cycle_duration;
  bool skip_missed_cycles;
  bool started = false;
  milliseconds current_cycle_start = 0;
};
