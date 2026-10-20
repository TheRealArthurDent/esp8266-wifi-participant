#pragma once

/**
 * \brief An interface defining a controller that depends on a WiFi connection.
 *
 * Implementing class will receive callbacks in case the WiFi connection is
 * established or lost. They come in turns: established, lost, established,
 * and so on, never twice in a row and never "lost" first.
 *
 * \warning The callbacks run in the ESP8266 SDK's system context, from the
 * \c WiFi.onStationMode* event handlers. They must return quickly: no
 * \c delay(), no \c yield(), no blocking I/O or long loops. To do real
 * work, set a flag in the callback and act on it in \c loop().
 */
class WifiDependent
{

public:
  /**
   * \brief Callback function informing about an established connection.
   *
   * Callback function that will be called by the WiFi connection's controller
   * in case a connection has been established.
   */
  virtual void onWifiConnectionEstablished() = 0;
  /**
   * \brief Callback function informing about a connection loss.
   *
   * Callback function that will be called by the WiFi connection's controller
   * in case a connection has been lost.
   */
  virtual void onWifiConnectionLost() {}

  // A polymorphic interface: copying or moving through it would slice.
  WifiDependent(const WifiDependent &) = delete;
  WifiDependent(WifiDependent &&) = delete;
  auto operator=(const WifiDependent &) -> WifiDependent & = delete;
  auto operator=(WifiDependent &&) -> WifiDependent & = delete;

protected:
  WifiDependent() = default;
  /**
   * \brief Protected and non-virtual: a \c WifiDependent is never deleted
   * through a pointer to this interface, so it needs no virtual destructor.
   */
  ~WifiDependent() = default;
};
