#pragma once

#include <string>

#include <ArduinoFake.h>
#include <unity.h>

/**
 * \brief Reports fakeit's verification failures as Unity test failures.
 *
 * Without it fakeit throws an exception that nothing catches, which aborts
 * the whole test program instead of failing the one test. Install it once
 * with \c installFakeitUnityAdapter() before \c UNITY_BEGIN().
 */
class FakeitUnityAdapter final : public fakeit::EventHandler
{
public:
  void handle(const fakeit::UnexpectedMethodCallEvent &event) override
  {
    fail(formatter.format(event), Unity.CurrentTestLineNumber);
  }

  void handle(const fakeit::SequenceVerificationEvent &event) override
  {
    fail(formatter.format(event), event.line());
  }

  void handle(const fakeit::NoMoreInvocationsVerificationEvent &event) override
  {
    fail(formatter.format(event), event.line());
  }

private:
  void fail(const std::string &text, UNITY_LINE_TYPE line)
  {
    message = text;
    UNITY_TEST_FAIL(line, message.c_str());
  }

  fakeit::DefaultEventFormatter formatter;
  std::string message;
};

/**
 * \brief Makes fakeit report its failures through Unity, see
 * \c FakeitUnityAdapter.
 */
inline auto installFakeitUnityAdapter() -> void
{
  static FakeitUnityAdapter adapter;
  Fakeit.setTestingFrameworkAdapter(adapter);
}
