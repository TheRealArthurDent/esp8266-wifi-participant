#pragma once

#include "ESP8266WiFi.h"
#include <list>
#include <vector>
#include "wifi-dependent.hpp"

class WifiConnection
{

  bool connected = false;
  std::vector<WifiDependent *> dependents;

  // Singleton "constructor"
public:
  static auto getInstance() -> WifiConnection &
  {
    static WifiConnection instance;
    return instance;
  }
  // hide constructor, copy constructor and = operator
private:
  WifiConnection() = default;             // hide default constructor
  WifiConnection(WifiConnection const &); // Don't implement.
  void operator=(WifiConnection const &); // Don't implement.

  // normal methods
public:
  auto isConnected() -> bool;
  void init();
  void init(std::list<WifiDependent *> dependents);
  // void addDependent
  // void removeDependent

private:
  void connect();
  void setConnected(bool connected);
  void notifyDependentsConnected();
  void notifyDependentsDisconnected();

  static void onWifiConnect(const WiFiEventStationModeGotIP &event);
  static void onWifiDisconnect(const WiFiEventStationModeDisconnected &event);
};
