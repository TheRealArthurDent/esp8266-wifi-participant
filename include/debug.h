#pragma once

/**
 * \file
 * \brief Serial logging that compiles away without \c DEBUG.
 *
 * With \c DEBUG defined (the \c nodemcuv2_DEBUG env), the macros print to
 * \c Serial. Without it, they expand to nothing, so their arguments are not
 * evaluated.
 */

#include <Arduino.h>

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

/**
 * \def DEBUG_PRINT(x)
 * \brief Prints \p x to \c Serial if \c DEBUG is defined.
 */

/**
 * \def DEBUG_PRINTLN(x)
 * \brief Prints \p x and a line break to \c Serial if \c DEBUG is defined.
 */

/**
 * \brief Starts the serial port for the \c DEBUG_PRINT macros.
 *
 * With \c DEBUG defined, opens \c Serial at \c MONITOR_SPEED baud and logs
 * that debugging is initialized. Without it, does nothing. Call it once at
 * the start of \c setup().
 */
void initDebugging();
