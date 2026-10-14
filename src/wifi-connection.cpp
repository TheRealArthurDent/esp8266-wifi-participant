#include "wifi-connection.hpp"
#include "led.h"

const int FLASH_INTERVAL = 250;

/**
 * An init method, that does not register any dependents.
 * It's currently private since no add or remove methods for
 * \c WifiDependents exist. Might become \c public in case that changes.
 */
void WifiConnection::init()
{
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINTLN(SECRET_WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(SECRET_HOSTNAME);
  wifi_connect_handler = WiFi.onStationModeGotIP(onWifiConnect);
  wifi_disconnect_handler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  connect();
}

/**
 * Initializes the controller while registering a collection of
 * \ref WifiDependent "WifiDependents", then connects to the WiFi.
 *
 * Blocks until the WiFi is connected, flashing the ESP LED meanwhile.
 *
 * \param dependents A collection of instances that depend on a WiFi
 * connection. The dependents will be informed about changes in the
 * connection's status. They must outlive the controller, which lives
 * until the program ends.
 */
void WifiConnection::init(std::list<WifiDependent *> dependents)
{
  notifier.setDependents(dependents);

  init();
}

/**
 * Returns the WiFi connection status.
 *
 * \return \c true in case the WiFi connection is established
 * or \c false otherwise.
 */
// cppcheck-suppress unusedFunction
bool WifiConnection::isConnected()
{
  return notifier.isConnected();
}

void WifiConnection::connect()
{

  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILT_IN_ESP, LOW);
    delay(FLASH_INTERVAL);
    digitalWrite(LED_BUILT_IN_ESP, HIGH);
    delay(FLASH_INTERVAL);
    DEBUG_PRINT(".");
  }
}

void WifiConnection::onWifiConnect(const WiFiEventStationModeGotIP &event)
{
  DEBUG_PRINT("Connected to WiFi ");
  DEBUG_PRINTLN(SECRET_WIFI_SSID);
  DEBUG_PRINT("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());

  getInstance().notifier.connectionEstablished();
}

void WifiConnection::onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
  DEBUG_PRINTLN("Disconnected from WiFi ");
  DEBUG_PRINTLN(SECRET_WIFI_SSID);

  getInstance().notifier.connectionLost();

  // Apparently this is not even required, since ESP8266WiFiClass tries to reconnect like a mad man anyway.
  // wifi_reconnect_timer.once(2, connectToWifi);
}
