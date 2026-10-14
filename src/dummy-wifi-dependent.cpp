#include "dummy-wifi-dependent.hpp"
#include "debug.h"

/**
 * Logs in case of an established WiFi connection.
 */
// cppcheck-suppress unusedFunction
void dummy_wifi_dependent::on_wifi_connection_established()
{
  DEBUG_PRINTLN("dummy_wifi_dependent connected!");
}

/**
 * Logs in case of an lost WiFi connection.
 */
// cppcheck-suppress unusedFunction
void dummy_wifi_dependent::on_wifi_connection_lost()
{
  DEBUG_PRINTLN("dummy_wifi_dependent disconnected!");
}
