#include "wifi-dependent-notifier.hpp"

// cppcheck-suppress unusedFunction
void wifi_dependent_notifier::set_dependents(const std::list<wifi_dependent *> &dependents_list)
{
  dependents.assign(dependents_list.begin(), dependents_list.end());
}

// cppcheck-suppress unusedFunction
void wifi_dependent_notifier::connection_established()
{
  connected = true;
  for (wifi_dependent *dependent : dependents)
  {
    dependent->on_wifi_connection_established();
  }
}

// cppcheck-suppress unusedFunction
void wifi_dependent_notifier::connection_lost()
{
  connected = false;
  for (wifi_dependent *dependent : dependents)
  {
    dependent->on_wifi_connection_lost();
  }
}

// cppcheck-suppress unusedFunction
auto wifi_dependent_notifier::is_connected() const -> bool
{
  return connected;
}
