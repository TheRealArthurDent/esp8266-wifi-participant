#pragma once

#include "interface.hpp"

#include <vector>

class Alert; // forward declaration because c++ things ...

class Alertable
{
    DECLARE_CLASS_AS_INTERFACE(Alertable)

public:
    virtual auto addAlert(Alert *alert) -> Alert * = 0;
    virtual auto removeAlert(Alert *alert) -> Alert * = 0;
    virtual auto stateChanged(Alert *alert) -> void = 0;
    virtual auto hasActiveAlerts() -> bool = 0;
    virtual auto getActiveAlerts() -> std::vector<Alert *> = 0;
    virtual auto getAllAlerts() -> std::vector<Alert *> = 0;
};
