#pragma once

class WifiDependent
{

public:
  virtual void onWifiConnectionEstablished() = 0;
  virtual void onWifiConnectionLost() {}
};
