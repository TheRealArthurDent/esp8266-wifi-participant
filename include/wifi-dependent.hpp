#pragma once

/**
 * \brief An interface defining a controller that depends on a WiFi connection.
 *
 * Implementing class will receive callbacks in case the WiFi connection is
 * established or lost.
 */
class wifi_dependent
{

public:
  /**
   * \brief Callback function informing about an established connection.
   *
   * Callback function that will be called by the WiFi connection's controller
   * in case a connection has been established.
   */
  virtual void on_wifi_connection_established() = 0;
  /**
   * \brief Callback function informing about a connection loss.
   *
   * Callback function that will be called by the WiFi connection's controller
   * in case a connection has been lost.
   */
  virtual void on_wifi_connection_lost() {}
};
