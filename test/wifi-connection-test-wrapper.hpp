#pragma once

#include "wifi-connection.hpp" // Include the original WifiConnection class

// Test wrapper class that extends WifiConnection and exposes private methods
class WifiConnectionTestWrapper : public WifiConnection
{
public:
  // Expose the private static methods for testing purposes
  using WifiConnection::onWifiConnect;
  using WifiConnection::onWifiDisconnect;

  // Optionally expose other private methods if necessary for testing
};
