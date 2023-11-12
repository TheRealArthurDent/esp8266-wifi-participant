#pragma once

#include "wifi-dependent.hpp"
#include "alert.hpp"

/**
 * \brief Dummy implementation of a WifiDependent.
 *
 * Just a dummy implementation of a WifiDependent that does nothing but
 * logging the respective events.
 */
class DummyWifiDependent : public WifiDependent
{
private:
  Alert *alert;

public:
  DummyWifiDependent() : alert(NULL){};
  DummyWifiDependent(Alert *alert);

  void onWifiConnectionEstablished() override;
  void onWifiConnectionLost() override;
};
