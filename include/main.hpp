#pragma once

/*
 * The main "class".
 *
 * A helper to get us into an object-oriented context.
 * Arduino isn't OO at all. It expects us to implement the static methods
 * setup and loop. This is what main does, thereby transferring us into
 * an OO context.
 */

void blinkLed();
