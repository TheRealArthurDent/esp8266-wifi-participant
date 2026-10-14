#include <ArduinoFake.h>
#include <unity.h>

#include "fakeit-unity.hpp"

#include "metronome.hpp"

using fakeit::Verify;
using fakeit::When;

namespace
{
const int cycle_duration = 1000;
} // namespace

void setUp()
{
  ArduinoFakeReset();
  When(Method(ArduinoFake(), delay)).AlwaysReturn();
}

void tearDown()
{
}

void waits_until_the_next_cycle_starts()
{
  When(Method(ArduinoFake(), millis)).Return(1200);
  metronome beat(cycle_duration);

  beat.wait_for_next_cycle();

  Verify(Method(ArduinoFake(), delay).Using(800)).Once();
}

void waits_a_whole_cycle_when_called_at_a_cycle_start()
{
  When(Method(ArduinoFake(), millis)).Return(3000);
  metronome beat(cycle_duration);

  beat.wait_for_next_cycle();

  Verify(Method(ArduinoFake(), delay).Using(1000)).Once();
}

void skips_a_missed_cycle_by_default()
{
  When(Method(ArduinoFake(), millis)).Return(1200, 3500);
  metronome beat(cycle_duration);

  beat.wait_for_next_cycle(); // cycle starts at 2000
  beat.wait_for_next_cycle(); // 3000 missed, next cycle starts at 4000

  Verify(Method(ArduinoFake(), delay).Using(800)).Once();
  Verify(Method(ArduinoFake(), delay).Using(500)).Once();
}

void returns_immediately_on_a_missed_cycle_when_not_skipping()
{
  When(Method(ArduinoFake(), millis)).Return(1200, 3500);
  metronome beat(cycle_duration, false);

  beat.wait_for_next_cycle(); // cycle starts at 2000
  beat.wait_for_next_cycle(); // 3000 missed, make it up right away

  Verify(Method(ArduinoFake(), delay).Using(800)).Once();
  Verify(Method(ArduinoFake(), delay).Using(0)).Once();
}

void returns_to_the_regular_cycles_after_making_up_a_missed_one()
{
  When(Method(ArduinoFake(), millis)).Return(1200, 3500, 3700);
  metronome beat(cycle_duration, false);

  beat.wait_for_next_cycle(); // cycle starts at 2000
  beat.wait_for_next_cycle(); // 3000 missed, made up at 3500
  beat.wait_for_next_cycle(); // back on track, next cycle starts at 4000

  Verify(Method(ArduinoFake(), delay).Using(800)).Once();
  Verify(Method(ArduinoFake(), delay).Using(0)).Once();
  Verify(Method(ArduinoFake(), delay).Using(300)).Once();
}

auto main() -> int
{
  install_fakeit_unity_adapter();
  UNITY_BEGIN();
  RUN_TEST(waits_until_the_next_cycle_starts);
  RUN_TEST(waits_a_whole_cycle_when_called_at_a_cycle_start);
  RUN_TEST(skips_a_missed_cycle_by_default);
  RUN_TEST(returns_immediately_on_a_missed_cycle_when_not_skipping);
  RUN_TEST(returns_to_the_regular_cycles_after_making_up_a_missed_one);
  return UNITY_END();
}
