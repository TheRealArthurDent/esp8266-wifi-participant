#pragma once

#include "wifi-dependent.hpp"

/**
 * \brief Dummy implementation of a wifi_dependent.
 *
 * Just a dummy implementation of a wifi_dependent that does nothing but
 * logging the respective events.
 */
class dummy_wifi_dependent : public wifi_dependent
{

public:
  void on_wifi_connection_established() override;
  void on_wifi_connection_lost() override;
};
