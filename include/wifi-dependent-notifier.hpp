#pragma once

#include <list>
#include <vector>
#include "wifi-dependent.hpp"

/**
 * \brief Keeps track of the WiFi connection state and informs the
 * registered \ref WifiDependent "WifiDependents" about its changes.
 *
 * The hardware-free part of \c WifiConnection: it knows nothing about the
 * ESP8266 WiFi stack, so it can be unit tested on the host.
 */
class WifiDependentNotifier
{
public:
  /**
   * \brief Replaces the registered dependents.
   *
   * \param dependents The instances to inform about changes of the
   * connection's status, in the order they will be informed. They must
   * outlive this notifier, or be replaced before they are destroyed.
   */
  void setDependents(const std::list<WifiDependent *> &dependents);

  /**
   * \brief Marks the connection as established and calls
   * WifiDependent::onWifiConnectionEstablished() on every dependent.
   *
   * Does nothing while the connection is already established, so the
   * dependents only hear about actual changes.
   */
  void connectionEstablished();

  /**
   * \brief Marks the connection as lost and calls
   * WifiDependent::onWifiConnectionLost() on every dependent.
   *
   * Does nothing unless the connection was established, so neither a failed
   * connection attempt nor a failed reconnect attempt after a loss notifies
   * the dependents.
   */
  void connectionLost();

  /**
   * \brief Returns the connection status.
   *
   * \return \c true after connectionEstablished(), \c false initially and
   * after connectionLost().
   */
  [[nodiscard]] auto isConnected() const -> bool;

private:
  bool connected = false;
  std::vector<WifiDependent *> dependents;
};
