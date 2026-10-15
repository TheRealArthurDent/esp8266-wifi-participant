#pragma once

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <list>
#include "wifi-dependent.hpp"
#include "wifi-dependent-notifier.hpp"

/**
 * \brief A controller for a WiFi connection.
 *
 * A singleton that will take care of your WiFi connection. It will try to
 * establish a connection, attempt reconnects in case of a connection loss
 * and inform registered \c WifiDependent s about successful connects as well
 * as connection losses by calling their
 * WifiDependent::onWifiConnectionEstablished() or
 * WifiDependent::onWifiConnectionLost() callback respectively.
 */
class WifiConnection
{
  WiFiEventHandler wifi_connect_handler;
  WiFiEventHandler wifi_disconnect_handler;
  // Ticker wifi_reconnect_timer;

  WifiDependentNotifier notifier;

  // Singleton "constructor"
public:
  /**
   * Gets the \c Singleton instance.
   */
  static auto getInstance() -> WifiConnection &
  {
    static WifiConnection instance;
    return instance;
  }
  // hide constructor, copy constructor and = operator
private:
  WifiConnection() = default;                 // hide default constructor
  WifiConnection(WifiConnection const &);     // Don't implement.
  WifiConnection(WifiConnection &&) noexcept; // Don't implement.
  void operator=(WifiConnection const &);     // Don't implement.
  void operator=(WifiConnection &&) noexcept; // Don't implement.
  ~WifiConnection() = default;                // Don't implement.

  // normal methods
public:
  auto isConnected() -> bool;
  void init(std::list<WifiDependent *> dependents);
  // void addDependent
  // void removeDependent

private:
  void init();
  void connect();

  static void onWifiConnect(const WiFiEventStationModeGotIP &event);
  static void onWifiDisconnect(const WiFiEventStationModeDisconnected &event);
};
