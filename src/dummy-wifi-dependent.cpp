#include "dummy-wifi-dependent.hpp"
#include "debug.h"

/**
 * Logs in case of an established WiFi connection.
 */
// cppcheck-suppress unusedFunction
void DummyWifiDependent::onWifiConnectionEstablished()
{
  DEBUG_PRINTLN("DummyWifiDependent connected!");
}

/**
 * Logs in case of a lost WiFi connection.
 */
// cppcheck-suppress unusedFunction
void DummyWifiDependent::onWifiConnectionLost()
{
  DEBUG_PRINTLN("DummyWifiDependent disconnected!");
}
