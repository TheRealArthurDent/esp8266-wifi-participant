#include "dummy-wifi-dependent.hpp"
#include "debug.h"

DummyWifiDependent::DummyWifiDependent(Alert *alert)
{
  this->alert = alert;
}

/**
 * Logs in case of an established WiFi connection.
 */
// cppcheck-suppress unusedFunction
void DummyWifiDependent::onWifiConnectionEstablished()
{
  DEBUG_PRINTLN("DummyWifiDependent connected!");
  if (alert)
  {
    alert->clear();
  }
}

/**
 * Logs in case of an lost WiFi connection.
 */
// cppcheck-suppress unusedFunction
void DummyWifiDependent::onWifiConnectionLost()
{
  DEBUG_PRINTLN("DummyWifiDependent disconnected!");
  if (alert)
  {
    alert->activate(std::string("Connection lost"));
  }
}