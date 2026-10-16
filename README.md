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
to secrets.ini. Its dummy values are enough to build and test.
`scripts/setup-dev.sh` does that for you and installs PlatformIO into a venv
(see below). Your real hostname and WiFi credentials only matter for flashing
a board and live outside the repo, see
[Keeping the WiFi credentials away from Claude](#keeping-the-wifi-credentials-away-from-claude).

## Flashing a board

The steps below use the PlatformIO CLI, the same tool the PlatformIO extension
for VS Code runs behind its upload button.

### 1. Prepare the checkout (once)

```sh
scripts/setup-dev.sh
```

This installs the pinned PlatformIO into `~/.venvs/esp8266-wifi-participant`
and tells you how to put `pio` on your `PATH`. It also creates `secrets.ini`
from the template if there is none; leave the dummy values in there.

Put your real hostname, WiFi SSID and WiFi password in a copy outside the
repo instead, readable only by you:

```sh
mkdir -p ~/.config/esp8266-wifi-participant
cp secrets.ini.template ~/.config/esp8266-wifi-participant/secrets.ini
chmod 600 ~/.config/esp8266-wifi-participant/secrets.ini
```

The ESP8266 only connects to 2.4 GHz networks with WPA2 (WPA3-only networks
don't work).

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
scripts/flash.sh                  # build and flash the default env (nodemcuv2_DEBUG)
scripts/flash.sh -e nodemcuv2     # the release build instead
scripts/flash.sh -t monitor       # flash, then open the serial log at 115200 baud
pio device monitor                # just the serial log, Ctrl+C to quit
```

`scripts/flash.sh` runs `pio run -t upload` with your real values from
`~/.config/esp8266-wifi-participant/secrets.ini` and builds into
`~/.cache/esp8266-wifi-participant/build` instead of `.pio/build`, because
the build output contains the values too. Further arguments go to `pio run`.
A plain `pio run -t upload` still works, but flashes the dummy values.

PlatformIO finds the port on its own; with several serial devices attached,
pick one with `--upload-port` (and `--port` for the monitor). The monitor
decodes ESP8266 exception backtraces to source lines when it runs together
with the upload (`-t monitor`), because only then does it find the firmware
it needs for that.

The monitor doesn't reset the board, so the boot messages are usually over
when it opens. Press the RST button on the NodeMCU to see the log from the
start: `Debugging initialized.`, `Connecting to …`, then the IP address once
connected.

## Keeping the WiFi credentials away from Claude

Claude Code builds and tests this project, so it can read everything in the
checkout. Its guard hook (`.claude/hooks/guard.sh`) and the permission rules
in `.claude/settings.json` stop accidental reads of the credentials, but they
match command text and can't stop every way of reading a file. A hard limit
comes from Claude Code's
[sandbox](https://code.claude.com/docs/en/sandboxing), which blocks reading
paths at the operating-system level for every command Claude runs.

That only works because the real values live outside the repo: the build
needs `secrets.ini`, so the sandbox can't hide that one, while only
`scripts/flash.sh`, which you run yourself, reads
`~/.config/esp8266-wifi-participant/`. To set it up once on Linux or WSL2
(not WSL1):

1. Move your real values out of the repo as in
   [step 1 of Flashing a board](#1-prepare-the-checkout-once), put the
   template's dummy values back into the repo's `secrets.ini`, and delete the
   old builds, which still contain the real password: `rm -rf .pio/build`.
2. Install the sandbox's dependencies: `sudo apt-get install bubblewrap socat`.
3. Enable the sandbox in `/etc/claude-code/managed-settings.json`. Managed
   settings take precedence over every other settings file, so neither
   Claude nor a `.claude/settings.local.json` can turn it off again:

   ```json
   {
     "permissions": {
       "deny": [
         "Read(~/.cache/esp8266-wifi-participant/**)",
         "Read(~/.config/esp8266-wifi-participant/**)"
       ]
     },
     "sandbox": {
       "allowUnsandboxedCommands": false,
       "enabled": true,
       "failIfUnavailable": true,
       "filesystem": {
         "allowWrite": ["~/.cache/pre-commit", "~/.platformio"],
         "denyRead": [
           "~/.cache/esp8266-wifi-participant",
           "~/.config/esp8266-wifi-participant"
         ]
       }
     }
   }
   ```

   `allowUnsandboxedCommands: false` removes Claude's way of retrying a
   failed command outside the sandbox, and `failIfUnavailable: true` stops
   it from running commands unsandboxed when bubblewrap is missing.
4. Check it: `/sandbox` in Claude Code shows whether the sandbox is active.
   Then let Claude run `pio run` and `pre-commit run --all-files`, which must
   work, and `cat ~/.config/esp8266-wifi-participant/secrets.ini`, which must
   fail. If PlatformIO needs more (another write path, network access to its
   registry), the failing command says so.

Commands you run yourself with `!` in Claude Code are not sandboxed. Don't
export `ESP8266_REAL_CONFIG` in the shell you start Claude from; only
`scripts/flash.sh` sets it.

## Documentation

[Source code documentation](https://therealarthurdent.github.io/esp8266-wifi-participant/index.html)
