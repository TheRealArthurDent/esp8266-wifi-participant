#include "alert.hpp"
#include "debug.h"

#include <algorithm>

Alert::Alert(std::string name) : name(name), active(false), message(ALL_GOOD), alertables(std::vector<Alertable *>(0))
{
    DEBUG_PRINTLN("New Alert");
}

Alert::Alert(std::string name, Alertable *alertable) : Alert(name)
{
    addAlertable(alertable);
}

// Alert::Alert(std::string name, std::list<Alertable *> alertable = std::list<Alertable *>(0))
// {
//     this->name = name;
//     std::for_each(alertable.begin(), alertable.end(), [this](Alertable *d)
//                   { alertables.push_back(d); });
// }

Alertable *Alert::addAlertable(Alertable *alertable)
{
    DEBUG_PRINTLN("Alert#addAlertable");
    if (std::find(alertables.begin(), alertables.end(), alertable) == alertables.end())
    {
        DEBUG_PRINTLN("Adding alertable");
        alertables.push_back(alertable);
        alertable->addAlert(this);
    }
    DEBUG_PRINT("Alert#addAlertable - alertables.size: ");
    DEBUG_PRINTLN(alertables.size());
    return alertable;
}

Alertable *Alert::removeAlertable(Alertable *alertable)
{
    auto it = std::find(alertables.begin(), alertables.end(), alertable);
    if (it != alertables.end())
    {
        alertables.erase(it);
        alertable->removeAlert(this);
    }
    return alertable;
}

void Alert::activate(std::string message)
{
    this->message = message;
    if (!active)
    {
        DEBUG_PRINT("Activating alert: ");
        DEBUG_PRINTLN(name.c_str());
        this->active = true;
        DEBUG_PRINT("Informing ");
        DEBUG_PRINT(alertables.size());
        DEBUG_PRINTLN(" handler(s).");
        std::for_each(alertables.begin(), alertables.end(), [this](Alertable *a)
                      { a->stateChanged(this); });
    }
}

void Alert::clear()
{
    this->message = ALL_GOOD;
    if (active)
    {
        this->active = false;
        std::for_each(alertables.begin(), alertables.end(), [this](Alertable *a)
                      { a->stateChanged(this); });
    }
}

bool Alert::isActive()
{
    return active;
}

std::string Alert::getName()
{
    return name;
}

std::string Alert::getMessage()
{
    return message;
}

std::string Alert::toString()
{
    return name + ": " + message;
}
