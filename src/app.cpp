#include "app.hpp"

#include "Arduino.h"
#include "led.h"
#include "wifi-connection.hpp"
#include "dummy-wifi-dependent.hpp"

App::App() : metronome(Metronome(LOOP_DURATION))
{
}

void App::setup()
{
    initDebugging();

    pinMode(led_built_in_ESP, OUTPUT);
    pinMode(led_built_in_Node, OUTPUT);
    digitalWrite(led_built_in_Node, HIGH);
    digitalWrite(led_built_in_ESP, HIGH);

    DummyWifiDependent dependent1;
    DummyWifiDependent dependent2;

    std::list<WifiDependent *> deps;
    deps.push_back(&dependent1);
    deps.push_back(&dependent2);

    WifiConnection::getInstance().init(deps);
}

void App::loop()
{
    blinkLed();
}

void App::blinkLed()
{
    digitalWrite(led_built_in_Node, LOW);
    delay(BLINK_INTERVAL);
    digitalWrite(led_built_in_Node, HIGH);

    metronome.waitForNextCycle();

    DEBUG_PRINT("Loop ended at: ");
    DEBUG_PRINTLN(millis());
}
