#include "wifi-connection.hpp"
#include "led.h"

const int flash_interval = 250;

/**
 * An init method, that does not register any dependents.
 * It's currently private since no add or remove methods for
 * \c wifi_dependent objects exist. Might become \c public in case that changes.
 */
void wifi_connection::init()
{
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINTLN(SECRET_WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(SECRET_HOSTNAME);
  wifi_connect_handler = WiFi.onStationModeGotIP(on_wifi_connect);
  wifi_disconnect_handler = WiFi.onStationModeDisconnected(on_wifi_disconnect);
  connect();
}

/**
 * Initializes the controller while registering a collection of
 * \c wifi_dependent s.
 *
 * \param dependents A collection of instances that depend on a WiFi
 * connection. The dependents will be informed about changes in the
 * connection's status.
 */
void wifi_connection::init(std::list<wifi_dependent *> dependents)
{
  notifier.set_dependents(dependents);

  init();
}

/**
 * Returns the WiFi connection status.
 *
 * \return \c true in case the WiFi connection is established
 * or \c false otherwise.
 */
// cppcheck-suppress unusedFunction
bool wifi_connection::is_connected()
{
  return notifier.is_connected();
}

void wifi_connection::connect()
{

  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILT_IN_ESP, LOW);
    delay(flash_interval);
    digitalWrite(LED_BUILT_IN_ESP, HIGH);
    delay(flash_interval);
    DEBUG_PRINT(".");
  }
}

void wifi_connection::on_wifi_connect(const WiFiEventStationModeGotIP &event)
{
  DEBUG_PRINT("Connected to WiFi ");
  DEBUG_PRINTLN(SECRET_WIFI_SSID);
  DEBUG_PRINT("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());

  get_instance().notifier.connection_established();
}

void wifi_connection::on_wifi_disconnect(const WiFiEventStationModeDisconnected &event)
{
  DEBUG_PRINTLN("Disconnected from WiFi ");
  DEBUG_PRINTLN(SECRET_WIFI_SSID);

  get_instance().notifier.connection_lost();

  // Apparently this is not even required, since ESP8266WiFiClass tries to reconnect like a mad man anyway.
  // wifi_reconnect_timer.once(2, connect_to_wifi);
}
