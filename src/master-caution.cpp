#include "master-caution.hpp"

#include "alert.hpp"

#include <algorithm>
#include "debug.h"

Alert *MasterCaution::addAlert(Alert *alert)
{
    DEBUG_PRINTLN("MasterCaution#addAlert");
    if (std::find(alerts.begin(), alerts.end(), alert) == alerts.end())
    {
        DEBUG_PRINTLN("Adding alert");
        alerts.push_back(alert);
        alert->addAlertable(this);
    }
    DEBUG_PRINT("MasterCaution#addAlert - alerts.size: ");
    DEBUG_PRINTLN(alerts.size());
    return alert;
}

Alert *MasterCaution::removeAlert(Alert *alert)
{
    auto it = std::find(alerts.begin(), alerts.end(), alert);
    if (it != alerts.end())
    {
        alerts.erase(it);
        alert->removeAlertable(this);
    }
    return alert;
}

void MasterCaution::stateChanged(Alert *alert)
{
    DEBUG_PRINT("MasterCaution#stateChanged - ");
    DEBUG_PRINT(alert->getName().c_str());
    DEBUG_PRINTLN(alert->isActive() ? " is in ALARM state." : " is silent.");
    if (alert->isActive())
    {
        DEBUG_PRINT("Alert ");
        DEBUG_PRINT(alert->getName().c_str());
        DEBUG_PRINT(" triggered: ");
        DEBUG_PRINTLN(alert->getMessage().c_str());
        active = true;
        acknowledged = false;
    }
    else if (!hasActiveAlerts())
    {
        DEBUG_PRINTLN("All alerts stopped triggering - deactivating MasterCaution");
        active = false;
        acknowledged = false;
    }
}

bool MasterCaution::hasActiveAlerts()
{
    bool result = false;
    for (auto &alert : alerts)
    {
        if (alert->isActive())
        {
            result = true;
            break;
        }
    }
    return result;
}

std::vector<Alert *> MasterCaution::getActiveAlerts()
{
    std::vector<Alert *> filteredResult;
    std::copy_if(alerts.begin(), alerts.end(), std::back_inserter(filteredResult), [this](Alert *a)
                 { return a->isActive(); });
    return filteredResult;
}

std::vector<Alert *> MasterCaution::getAllAlerts()
{
    std::vector<Alert *> result;
    std::copy(alerts.begin(), alerts.end(), std::back_inserter(result));
    return result;
}

void MasterCaution::acknowledge()
{
    acknowledged = true;
}

bool MasterCaution::isAcknowledged()
{
    return acknowledged;
}

bool MasterCaution::isAlarming()
{
    return active && !acknowledged;
}