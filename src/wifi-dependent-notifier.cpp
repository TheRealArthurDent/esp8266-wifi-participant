#include "wifi-dependent-notifier.hpp"

// cppcheck-suppress unusedFunction
void WifiDependentNotifier::setDependents(const std::list<WifiDependent *> &dependents_list)
{
  dependents.assign(dependents_list.begin(), dependents_list.end());
}

// cppcheck-suppress unusedFunction
void WifiDependentNotifier::connectionEstablished()
{
  if (connected)
  {
    return;
  }
  connected = true;
  for (WifiDependent *dependent : dependents)
  {
    dependent->onWifiConnectionEstablished();
  }
}

// cppcheck-suppress unusedFunction
void WifiDependentNotifier::connectionLost()
{
  if (!connected)
  {
    return;
  }
  connected = false;
  for (WifiDependent *dependent : dependents)
  {
    dependent->onWifiConnectionLost();
  }
}

// cppcheck-suppress unusedFunction
auto WifiDependentNotifier::isConnected() const -> bool
{
  return connected;
}
