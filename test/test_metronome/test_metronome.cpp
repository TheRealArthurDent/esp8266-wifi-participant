#include <ArduinoFake.h>
#include <unity.h>

#include "fakeit-unity.hpp"

#include "metronome.hpp"

using fakeit::Verify;
using fakeit::When;

namespace
{
const int CYCLE_DURATION = 1000;
} // namespace

void setUp()
{
  ArduinoFakeReset();
  When(Method(ArduinoFake(), delay)).AlwaysReturn();
}

void tearDown()
{
}

void waitsUntilTheNextCycleStarts()
{
  When(Method(ArduinoFake(), millis)).Return(1200);
  Metronome metronome(CYCLE_DURATION);

  metronome.waitForNextCycle();

  Verify(Method(ArduinoFake(), delay).Using(800)).Once();
}

void waitsAWholeCycleWhenCalledAtACycleStart()
{
  When(Method(ArduinoFake(), millis)).Return(3000);
  Metronome metronome(CYCLE_DURATION);

  metronome.waitForNextCycle();

  Verify(Method(ArduinoFake(), delay).Using(1000)).Once();
}

void skipsAMissedCycleByDefault()
{
  When(Method(ArduinoFake(), millis)).Return(1200, 3500);
  Metronome metronome(CYCLE_DURATION);

  metronome.waitForNextCycle(); // cycle starts at 2000
  metronome.waitForNextCycle(); // 3000 missed, next cycle starts at 4000

  Verify(Method(ArduinoFake(), delay).Using(800)).Once();
  Verify(Method(ArduinoFake(), delay).Using(500)).Once();
}

void returnsImmediatelyOnAMissedCycleWhenNotSkipping()
{
  When(Method(ArduinoFake(), millis)).Return(1200, 3500);
  Metronome metronome(CYCLE_DURATION, false);

  metronome.waitForNextCycle(); // cycle starts at 2000
  metronome.waitForNextCycle(); // 3000 missed, make it up right away

  Verify(Method(ArduinoFake(), delay).Using(800)).Once();
  Verify(Method(ArduinoFake(), delay).Using(0)).Once();
}

void returnsToTheRegularCyclesAfterMakingUpAMissedOne()
{
  When(Method(ArduinoFake(), millis)).Return(1200, 3500, 3700);
  Metronome metronome(CYCLE_DURATION, false);

  metronome.waitForNextCycle(); // cycle starts at 2000
  metronome.waitForNextCycle(); // 3000 missed, made up at 3500
  metronome.waitForNextCycle(); // back on track, next cycle starts at 4000

  Verify(Method(ArduinoFake(), delay).Using(800)).Once();
  Verify(Method(ArduinoFake(), delay).Using(0)).Once();
  Verify(Method(ArduinoFake(), delay).Using(300)).Once();
}

auto main() -> int
{
  installFakeitUnityAdapter();
  UNITY_BEGIN();
  RUN_TEST(waitsUntilTheNextCycleStarts);
  RUN_TEST(waitsAWholeCycleWhenCalledAtACycleStart);
  RUN_TEST(skipsAMissedCycleByDefault);
  RUN_TEST(returnsImmediatelyOnAMissedCycleWhenNotSkipping);
  RUN_TEST(returnsToTheRegularCyclesAfterMakingUpAMissedOne);
  return UNITY_END();
}
