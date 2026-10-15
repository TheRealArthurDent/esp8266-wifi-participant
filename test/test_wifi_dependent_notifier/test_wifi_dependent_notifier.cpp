#include <ArduinoFake.h>
#include <unity.h>

#include "fakeit-unity.hpp"

#include "wifi-dependent-notifier.hpp"

using fakeit::Mock;
using fakeit::Verify;
using fakeit::VerifyNoOtherInvocations;
using fakeit::When;

namespace
{
/// A \c WifiDependent mock that accepts both callbacks.
class DependentMock
{
public:
  DependentMock()
  {
    When(Method(mock, onWifiConnectionEstablished)).AlwaysReturn();
    When(Method(mock, onWifiConnectionLost)).AlwaysReturn();
  }

  auto get() -> WifiDependent *
  {
    return &mock.get();
  }

  auto operator()() -> Mock<WifiDependent> &
  {
    return mock;
  }

private:
  Mock<WifiDependent> mock;
};
} // namespace

void setUp()
{
  ArduinoFakeReset();
}

void tearDown()
{
}

void isNotConnectedInitially()
{
  const WifiDependentNotifier notifier;

  TEST_ASSERT_FALSE(notifier.isConnected());
}

void isConnectedOnceTheConnectionIsEstablished()
{
  WifiDependentNotifier notifier;

  notifier.connectionEstablished();

  TEST_ASSERT_TRUE(notifier.isConnected());
}

void isNotConnectedOnceTheConnectionIsLost()
{
  WifiDependentNotifier notifier;

  notifier.connectionEstablished();
  notifier.connectionLost();

  TEST_ASSERT_FALSE(notifier.isConnected());
}

void notifiesEachDependentOnceWhenTheConnectionIsEstablished()
{
  DependentMock first;
  DependentMock second;
  WifiDependentNotifier notifier;
  notifier.setDependents({first.get(), second.get()});

  notifier.connectionEstablished();

  Verify(Method(first(), onWifiConnectionEstablished)).Once();
  Verify(Method(second(), onWifiConnectionEstablished)).Once();
  VerifyNoOtherInvocations(first(), second());
}

void notifiesEachDependentOnceWhenTheConnectionIsLost()
{
  DependentMock first;
  DependentMock second;
  WifiDependentNotifier notifier;
  notifier.setDependents({first.get(), second.get()});

  notifier.connectionLost();

  Verify(Method(first(), onWifiConnectionLost)).Once();
  Verify(Method(second(), onWifiConnectionLost)).Once();
  VerifyNoOtherInvocations(first(), second());
}

void notifiesDependentsInTheOrderTheyWereGiven()
{
  DependentMock first;
  DependentMock second;
  WifiDependentNotifier notifier;
  notifier.setDependents({first.get(), second.get()});

  notifier.connectionEstablished();

  Verify(Method(first(), onWifiConnectionEstablished), Method(second(), onWifiConnectionEstablished));
}

void settingDependentsReplacesTheEarlierOnes()
{
  DependentMock replaced;
  DependentMock current;
  WifiDependentNotifier notifier;
  notifier.setDependents({replaced.get()});

  notifier.setDependents({current.get()});
  notifier.connectionEstablished();
  notifier.connectionLost();

  VerifyNoOtherInvocations(replaced());
  Verify(Method(current(), onWifiConnectionEstablished)).Once();
  Verify(Method(current(), onWifiConnectionLost)).Once();
}

void notifiesNobodyWithoutDependents()
{
  WifiDependentNotifier notifier;

  notifier.connectionEstablished();
  notifier.connectionLost();

  TEST_ASSERT_FALSE(notifier.isConnected());
}

auto main() -> int
{
  installFakeitUnityAdapter();
  UNITY_BEGIN();

  RUN_TEST(isNotConnectedInitially);
  RUN_TEST(isConnectedOnceTheConnectionIsEstablished);
  RUN_TEST(isNotConnectedOnceTheConnectionIsLost);
  RUN_TEST(notifiesEachDependentOnceWhenTheConnectionIsEstablished);
  RUN_TEST(notifiesEachDependentOnceWhenTheConnectionIsLost);
  RUN_TEST(notifiesDependentsInTheOrderTheyWereGiven);
  RUN_TEST(settingDependentsReplacesTheEarlierOnes);
  RUN_TEST(notifiesNobodyWithoutDependents);

  return UNITY_END();
}
