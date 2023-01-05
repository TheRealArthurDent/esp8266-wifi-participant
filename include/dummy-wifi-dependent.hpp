#pragma once

#include "wifi-dependent.hpp"

class DummyWifiDependent : public WifiDependent
{

public:
  void onWifiConnectionEstablished() override;
  void onWifiConnectionLost() override;
};
