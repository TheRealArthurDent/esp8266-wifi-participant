#pragma once

#include "interface.hpp"

/**
 * \brief The main application.
 *
 * Apparently the Arduino guys dislike OO design. I know this stuff runs on a
 * rather tiny controller, but c'mon!
 * So here we go: In your \c main.cpp create an instance of this fella and
 * simply delegate Arduino's \c setup and \c loop calls to it. So now we're in
 * the OO world. The rest is up to you.
 */
class ArduinoCallable
{
    DECLARE_CLASS_AS_INTERFACE(ArduinoCallable)

public:
    /// @brief Arduino's <a href="https://www.arduino.cc/reference/en/language/structure/sketch/setup/">setup</a> function.
    virtual auto setup() -> void = 0;
    /// @brief Arduino's <a href="https://www.arduino.cc/reference/en/language/structure/sketch/loop/">loop</a> function.
    virtual auto loop() -> void = 0;
};