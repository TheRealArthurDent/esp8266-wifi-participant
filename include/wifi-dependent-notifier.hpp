#pragma once

#include <list>
#include <vector>
#include "wifi-dependent.hpp"

/**
 * \brief Keeps track of the WiFi connection state and informs the
 * registered \c wifi_dependent s about its changes.
 *
 * The hardware-free part of \c wifi_connection: it knows nothing about the
 * ESP8266 WiFi stack, so it can be unit tested on the host.
 */
class wifi_dependent_notifier
{
public:
  /**
   * \brief Replaces the registered dependents.
   *
   * \param dependents The instances to inform about changes of the
   * connection's status, in the order they will be informed. They must
   * outlive this notifier, or be replaced before they are destroyed.
   */
  void set_dependents(const std::list<wifi_dependent *> &dependents);

  /**
   * \brief Marks the connection as established and calls
   * wifi_dependent::on_wifi_connection_established() on every dependent.
   */
  void connection_established();

  /**
   * \brief Marks the connection as lost and calls
   * wifi_dependent::on_wifi_connection_lost() on every dependent.
   */
  void connection_lost();

  /**
   * \brief Returns the connection status.
   *
   * \return \c true after connection_established(), \c false initially and
   * after connection_lost().
   */
  [[nodiscard]] auto is_connected() const -> bool;

private:
  bool connected = false;
  std::vector<wifi_dependent *> dependents;
};
