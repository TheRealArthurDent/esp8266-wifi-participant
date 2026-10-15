# Software-in-the-loop (SIL) tests

The second of three test levels (#143): Unity unit tests in `test/`, these
software-in-the-loop tests, and hardware in the loop (HIL, #139). The real
firmware (`src/`) is compiled together with the real ESP8266 Arduino core
libraries, including `ESP8266WiFi`, into a native Linux program by the core's
own [host emulation](https://github.com/esp8266/Arduino/tree/master/tests/host).
The scenarios run it for a few seconds and check its serial output.

```sh
test-sil/run.sh               # all scenarios
test-sil/run.sh wifi_events   # one scenario
```

The first run clones the core (pinned to the version PlatformIO builds with)
into `.pio/sil/` and builds it, which takes about a minute. Logs of
the build and of every scenario end up in `.pio/sil/logs/`.

Needs Linux, `git`, `make` and `g++`. No PlatformIO, no board, no Docker.

## How it works

- Serial output (UART0) goes to stdout, so `DEBUG_PRINT` output can be checked
  like on the serial monitor. `digitalWrite()` and friends are only logged.
- The emulation never fires WiFi events. `wifi-events.cpp` fires `got_ip` and
  `disconnected` events at the times given in `WIFI_EVENTS`, e.g.
  `WIFI_EVENTS="got_ip@1000,disconnected@2000"`, while the firmware waits in
  `delay()`, like the SDK does on the chip.
- `WiFi.status()` follows these events: `WL_IDLE_STATUS` until the first one,
  `WL_CONNECTED` after `got_ip`, and back to "connecting"
  (`WL_DISCONNECTED`) after `disconnected`. So, as on the board,
  `WifiConnection::connect()` waits in `setup()` until `got_ip` is fired, and
  every scenario that needs `loop()` has to fire it.
- Everything is built with AddressSanitizer, so use-after-free and stale stack
  pointers crash the run instead of passing silently.
- `core-host-emulation.patch` makes a few small changes to the core's
  emulation: it builds with glibc 2.38 and later (which ship `strlcpy`),
  `delay()` calls a hook (`mock_sdk_task()`) that `wifi-events.cpp`
  implements, the WiFi connect status is a variable
  (`mock_station_connect_status`) instead of always "got IP", and `delay()`
  ends the program on Ctrl-C (or `timeout`'s `SIGINT`), which the emulation
  otherwise only checks between `loop()` calls.
- Secrets are fixed dummy values (`emulated-ssid`, …); `secrets.ini` isn't
  read.

## Limits

- `WiFi.localIP()` returns the host's own address, not the one in the event.
- Time is real time: a scenario that needs 5 s of firmware time takes 5 s.

## Scenarios

Scenarios are shell functions `scenario_<name>` in `run.sh`. A scenario that
fails because of a known bug is listed in `known_failures`, with its issue;
the run stays green while it fails and turns red once it passes, so the fix
removes it from the list.

- `boots`: `setup()` runs and the loop keeps going. Passes.
- `waits_for_wifi`: without `got_ip`, `connect()` keeps flashing the LED and
  printing dots, and `loop()` never runs. Passes.
- `wifi_events`: connect, disconnect and reconnect reach both dependents.
  Passes.
