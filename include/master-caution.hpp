#pragma once

#include "alertable.hpp"

#include <vector>

class MasterCaution : public Alertable
{
private:
    std::vector<Alert *> alerts;
    bool active;
    bool acknowledged;

public:
    MasterCaution() : alerts(std::vector<Alert *>(0)), active(false), acknowledged(false){};

    auto addAlert(Alert *alert) -> Alert * override;
    auto removeAlert(Alert *alert) -> Alert * override;
    auto stateChanged(Alert *alert) -> void override;
    auto hasActiveAlerts() -> bool override;
    auto getActiveAlerts() -> std::vector<Alert *> override;
    auto getAllAlerts() -> std::vector<Alert *> override;

    auto acknowledge() -> void;
    auto isAcknowledged() -> bool;
    auto isAlarming() -> bool;
};
