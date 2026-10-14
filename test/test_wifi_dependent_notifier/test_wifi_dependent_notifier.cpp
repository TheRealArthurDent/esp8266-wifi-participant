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
/// A \c wifi_dependent mock that accepts both callbacks.
class dependent_mock
{
public:
  dependent_mock()
  {
    When(Method(mock, on_wifi_connection_established)).AlwaysReturn();
    When(Method(mock, on_wifi_connection_lost)).AlwaysReturn();
  }

  auto get() -> wifi_dependent *
  {
    return &mock.get();
  }

  auto operator()() -> Mock<wifi_dependent> &
  {
    return mock;
  }

private:
  Mock<wifi_dependent> mock;
};
} // namespace

void setUp()
{
  ArduinoFakeReset();
}

void tearDown()
{
}

void is_not_connected_initially()
{
  const wifi_dependent_notifier notifier;

  TEST_ASSERT_FALSE(notifier.is_connected());
}

void is_connected_once_the_connection_is_established()
{
  wifi_dependent_notifier notifier;

  notifier.connection_established();

  TEST_ASSERT_TRUE(notifier.is_connected());
}

void is_not_connected_once_the_connection_is_lost()
{
  wifi_dependent_notifier notifier;

  notifier.connection_established();
  notifier.connection_lost();

  TEST_ASSERT_FALSE(notifier.is_connected());
}

void notifies_each_dependent_once_when_the_connection_is_established()
{
  dependent_mock first;
  dependent_mock second;
  wifi_dependent_notifier notifier;
  notifier.set_dependents({first.get(), second.get()});

  notifier.connection_established();

  Verify(Method(first(), on_wifi_connection_established)).Once();
  Verify(Method(second(), on_wifi_connection_established)).Once();
  VerifyNoOtherInvocations(first(), second());
}

void notifies_each_dependent_once_when_the_connection_is_lost()
{
  dependent_mock first;
  dependent_mock second;
  wifi_dependent_notifier notifier;
  notifier.set_dependents({first.get(), second.get()});

  notifier.connection_lost();

  Verify(Method(first(), on_wifi_connection_lost)).Once();
  Verify(Method(second(), on_wifi_connection_lost)).Once();
  VerifyNoOtherInvocations(first(), second());
}

void notifies_dependents_in_the_order_they_were_given()
{
  dependent_mock first;
  dependent_mock second;
  wifi_dependent_notifier notifier;
  notifier.set_dependents({first.get(), second.get()});

  notifier.connection_established();

  Verify(Method(first(), on_wifi_connection_established), Method(second(), on_wifi_connection_established));
}

void setting_dependents_replaces_the_earlier_ones()
{
  dependent_mock replaced;
  dependent_mock current;
  wifi_dependent_notifier notifier;
  notifier.set_dependents({replaced.get()});

  notifier.set_dependents({current.get()});
  notifier.connection_established();
  notifier.connection_lost();

  VerifyNoOtherInvocations(replaced());
  Verify(Method(current(), on_wifi_connection_established)).Once();
  Verify(Method(current(), on_wifi_connection_lost)).Once();
}

void notifies_nobody_without_dependents()
{
  wifi_dependent_notifier notifier;

  notifier.connection_established();
  notifier.connection_lost();

  TEST_ASSERT_FALSE(notifier.is_connected());
}

auto main() -> int
{
  install_fakeit_unity_adapter();
  UNITY_BEGIN();

  RUN_TEST(is_not_connected_initially);
  RUN_TEST(is_connected_once_the_connection_is_established);
  RUN_TEST(is_not_connected_once_the_connection_is_lost);
  RUN_TEST(notifies_each_dependent_once_when_the_connection_is_established);
  RUN_TEST(notifies_each_dependent_once_when_the_connection_is_lost);
  RUN_TEST(notifies_dependents_in_the_order_they_were_given);
  RUN_TEST(setting_dependents_replaces_the_earlier_ones);
  RUN_TEST(notifies_nobody_without_dependents);

  return UNITY_END();
}
