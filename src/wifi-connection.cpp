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
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  connect();
}

/**
 * Initializes the controller while registering a collection of
 * \c WifiDependent s.
 *
 * \param dependents A collection of instances that depend on a WiFi
 * connection. The dependents will be informed about changes in the
 * connection's status.
 */
void WifiConnection::init(std::list<WifiDependent *> dependentsList)
{
  dependents.clear();
  std::for_each(dependentsList.begin(), dependentsList.end(), [this](WifiDependent *d)
                { dependents.push_back(d); });

  init();
}

/**
 * Returns the WiFi connection status.
 *
 * \return \c true in case the WiFi connection is established
 * or \c false otherwise.
 */
// cppcheck-suppress unusedFunction
auto WifiConnection::isConnected() -> const bool
{
  return connected;
}

void WifiConnection::setConnected(bool conn)
{
  connected = conn;
}

void WifiConnection::notifyDependentsConnected()
{
  std::for_each(dependents.begin(), dependents.end(), [](WifiDependent *d)
                { d->onWifiConnectionEstablished(); });
}

void WifiConnection::notifyDependentsDisconnected()
{
  std::for_each(dependents.begin(), dependents.end(), [](WifiDependent *d)
                { d->onWifiConnectionLost(); });
}

void WifiConnection::connect()
{

  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(led_built_in_ESP, LOW);
    delay(FLASH_INTERVAL);
    digitalWrite(led_built_in_ESP, HIGH);
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

  getInstance().setConnected(true);
  getInstance().notifyDependentsConnected();
}

void WifiConnection::onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
  DEBUG_PRINTLN("Disconnected from WiFi ");
  DEBUG_PRINTLN(SECRET_WIFI_SSID);

  getInstance().setConnected(false);
  getInstance().notifyDependentsDisconnected();

  // Apparently this is not even required, since ESP8266WiFiClass tries to reconnect like a mad man anyway.
  // wifiReconnectTimer.once(2, connectToWifi);
}
