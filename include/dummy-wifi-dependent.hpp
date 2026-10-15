#pragma once

#include "wifi-dependent.hpp"

/**
 * \brief Dummy implementation of a WifiDependent.
 *
 * Just a dummy implementation of a WifiDependent that does nothing but
 * logging the respective events.
 */
class DummyWifiDependent : public WifiDependent
{

public:
  void onWifiConnectionEstablished() override;
  void onWifiConnectionLost() override;
};
