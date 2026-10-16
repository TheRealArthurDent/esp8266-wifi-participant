# esp8266-wifi-participant

License

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Status

[![CodeQL](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/codeql.yaml/badge.svg)](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/codeql.yaml)
[![PlatformIO Code Check](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/platformio-check.yaml/badge.svg)](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/platformio-check.yaml)
[![PlatformIO CI](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/platformio-ci.yaml/badge.svg)](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/platformio-ci.yaml)
[![CPP Lint status](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/cpp-lint.yaml/badge.svg)](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/cpp-lint.yaml)
[![Spell Check status](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/spell-check.yaml/badge.svg)](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/spell-check.yaml)
[![Review dog](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/reviewdog.yaml/badge.svg)](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/reviewdog.yaml)

## Compatibility with Arduino IDE

[![Check Arduino status](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/check-arduino.yaml/badge.svg)](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/check-arduino.yaml)
[![Compile Sketch status](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/compile-sketch.yaml/badge.svg)](https://github.com/TheRealArthurDent/esp8266-wifi-participant/actions/workflows/compile-sketch.yaml)

Always fresh

[![Renovate enabled](https://img.shields.io/badge/renovate-enabled-brightgreen.svg)](https://renovatebot.com/)

Open topics

[![GitHub issues](https://img.shields.io/github/issues/TheRealArthurDent/esp8266-wifi-participant.svg)](https://GitHub.com/TheRealArthurDent/esp8266-wifi-participant/issues/)
[![GitHub pull-requests](https://img.shields.io/github/issues-pr/TheRealArthurDent/esp8266-wifi-participant.svg)](https://GitHub.com/TheRealArthurDent/esp8266-wifi-participant/pull/)

## Description

A tiny sample project that makes your ESP8266 a participant in a WiFi network.
On startup your board will try to connect to the specified WiFi network and will
blink the built in ESP LED (GPIO 2) at 2 revolution per second while doing so.
Once successfully connected the node LED (GPIO 16) will blink at 1 revolution
per 5 seconds.
If started with "#define DEBUG" it will log the progress via the Serial port.

## How-to-use

To be able to compile the project locally, copy the secrets.ini.template
to secrets.ini and adjust the values within said file.
`scripts/setup-dev.sh` does that for you and installs PlatformIO into a venv
(see below).

## Flashing a board

The steps below use the PlatformIO CLI, the same tool the PlatformIO extension
for VS Code runs behind its upload button.

### 1. Prepare the checkout (once)

```sh
scripts/setup-dev.sh
```

This installs the pinned PlatformIO into `~/.venvs/esp8266-wifi-participant`
and tells you how to put `pio` on your `PATH`. It also creates `secrets.ini`
from the template if there is none: put your hostname, WiFi SSID and WiFi
password in there. The ESP8266 only connects to 2.4 GHz networks with WPA2
(WPA3-only networks don't work).

### 2. Connect the board

Plug the NodeMCU in via USB. Its USB-to-serial chip is a CP2102 (some clones
use a CH340).

- **Linux**: the board shows up as `/dev/ttyUSB0`. Your user must be in the
  `dialout` group to open it (`sudo usermod -aG dialout $USER`, then log in
  again).
- **Windows**: if the Device Manager lists the board with an error, install
  the Silicon Labs "CP210x Universal Windows Driver" (or the CH340 driver).
  The board then gets a COM port.
- **WSL2**: WSL doesn't see USB devices by itself. Install the driver on
  Windows as above, then pass the board through with
  [usbipd-win](https://github.com/dorssel/usbipd-win), in a Windows terminal:

  ```powershell
  winget install usbipd                    # once
  usbipd list                              # note the BUSID of the CP2102
  usbipd bind --busid <BUSID>              # once, as administrator
  usbipd attach --wsl --busid <BUSID> --auto-attach
  ```

  With `--auto-attach` the command keeps running and attaches the board again
  whenever it is replugged. In WSL it is then `/dev/ttyUSB0`, with the same
  `dialout` group as on Linux.

### 3. Build, flash and watch

```sh
pio run -t upload                 # build and flash the default env (nodemcuv2_DEBUG)
pio run -e nodemcuv2 -t upload    # the release build instead
pio device monitor                # serial log at 115200 baud, Ctrl+C to quit
pio run -t upload -t monitor      # flash, then open the monitor right away
```

PlatformIO finds the port on its own; with several serial devices attached,
pick one with `--upload-port` (and `--port` for the monitor). The monitor
decodes ESP8266 exception backtraces to source lines.

The monitor doesn't reset the board, so the boot messages are usually over
when it opens. Press the RST button on the NodeMCU to see the log from the
start: `Debugging initialized.`, `Connecting to …`, then the IP address once
connected.

## Documentation

[Source code documentation](https://therealarthurdent.github.io/esp8266-wifi-participant/index.html)
