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
 * and inform registered \c wifi_dependent s about successful connects as well
 * as connection losses by calling their
 * wifi_dependent::on_wifi_connection_established() or
 * wifi_dependent::on_wifi_connection_lost() callback respectively.
 */
class wifi_connection
{
  WiFiEventHandler wifi_connect_handler;
  WiFiEventHandler wifi_disconnect_handler;
  // Ticker wifi_reconnect_timer;

  wifi_dependent_notifier notifier;

  // Singleton "constructor"
public:
  /**
   * Gets the \c Singleton instance.
   */
  static auto get_instance() -> wifi_connection &
  {
    static wifi_connection instance;
    return instance;
  }
  // hide constructor, copy constructor and = operator
private:
  wifi_connection() = default;                  // hide default constructor
  wifi_connection(wifi_connection const &);     // Don't implement.
  wifi_connection(wifi_connection &&) noexcept; // Don't implement.
  void operator=(wifi_connection const &);      // Don't implement.
  void operator=(wifi_connection &&) noexcept;  // Don't implement.
  ~wifi_connection() = default;                 // Don't implement.

  // normal methods
public:
  auto is_connected() -> bool;
  void init(std::list<wifi_dependent *> dependents);
  // void add_dependent
  // void remove_dependent

private:
  void init();
  void connect();

  static void on_wifi_connect(const WiFiEventStationModeGotIP &event);
  static void on_wifi_disconnect(const WiFiEventStationModeDisconnected &event);
};
