#pragma once

/**
 * \file
 * \brief GPIO numbers of the NodeMCU's built-in LEDs.
 *
 * Both LEDs are active low: \c LOW switches them on.
 */

/// The ESP-12 module's LED, which flashes while connecting to the WiFi.
#define LED_BUILT_IN_ESP 2
/// The NodeMCU board's LED, which blinks once per loop.
#define LED_BUILT_IN_NODE 16
