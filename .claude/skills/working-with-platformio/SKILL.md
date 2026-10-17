---
name: working-with-platformio
description: Use when building, testing, checking or changing the PlatformIO setup of esp8266-wifi-participant - running pio run, pio test or pio check, editing platformio.ini, adding a library, bumping the espressif8266 platform, or reading PlatformIO errors and warnings.
---

# Working with PlatformIO

## Overview

`platformio.ini` is the source of truth for the board builds and the unit
tests, but not for everything: the SIL tests, the Arduino IDE compile and
Renovate each mirror parts of it. Most mistakes here are a change made in
one place but not the others, or a command that "passes" without running
anything.

## Envs

Read the current list instead of assuming it: `pio project config`, or
`grep '^\[env:' platformio.ini`.

| Env | What | Command |
| --- | --- | --- |
| `nodemcuv2_DEBUG` | default board build, `-D DEBUG` | `pio run` |
| `nodemcuv2` | release board build | `pio run -e nodemcuv2` |
| `unit_test*` | Unity tests on the host (`native`) | `pio test -e <env>` |

`nodemcuv2_DEBUG` copies `platform`, `board` and `framework` from
`nodemcuv2` via `${env:nodemcuv2.…}`; anything else (like `lib_deps`) must be
added to both, or referenced the same way.

## Tests

- **Plain `pio test` runs nothing and exits 0** ("0 test cases"), because the
  board envs set `test_ignore = *`. Always name every `unit_test*` env:
  `pio test -e unit_test` (plus `-e unit_test_DEBUG` where it exists).
- A test suite only links the sources listed in `build_src_filter` of
  `[env:unit_test]`. A new hardware-free `src/foo.cpp` under test gets
  `+<foo.cpp>` there, or the test fails with undefined references.
- `pio test -e unit_test -f test_foo` runs one suite.
- `test-sil/run.sh` doesn't read `platformio.ini` at all. It compiles every
  `src/*.cpp` with its own flags (`-DDEBUG`, `MONITOR_SPEED`, dummy
  secrets). A new build flag or library must be mirrored there too.

## Static analysis: `pio check`

- CI runs `pio check -e nodemcuv2_DEBUG`. It fails only on **high**
  severity defects; the dozens of medium/low findings on `main` pass.
  Compare the findings for the files you touched before and after your
  change (`pio check -e nodemcuv2_DEBUG --severity medium`) rather than
  trusting the exit code.
- `check_flags` in `platformio.ini` passes `--config-file=.clang-tidy`;
  without it PlatformIO calls clang-tidy with `--checks=*`, which overrides
  the `Checks:` list and floods the report with `llvmlibc-*`, `fuchsia-*`
  and `altera-*` noise. PlatformIO's clang-tidy is 15, cpp-linter's in CI
  is 21, so they still disagree on some findings (15 reports
  `cppcoreguidelines-pro-type-member-init` on `std::vector` and
  `std::shared_ptr` members, 21 calls the `{}` that silences it redundant;
  follow 21). To run them the way cpp-linter does, after
  `pio pkg install -e unit_test`:
  `clang-tidy <files> -- -Iinclude/ -Itest/
  -I.pio/libdeps/unit_test/Unity/src -I.pio/libdeps/unit_test/ArduinoFake/src`
- Formatting: CI's cpp-linter uses clang-format 21, the session has 18, and
  they disagree. Check with 21:
  `python3 -m pip install --target ~/cf21 clang-format==21.1.8`, then
  `~/cf21/clang_format/data/bin/clang-format --dry-run --Werror <files>`.

## Changing versions and libraries

Everything is pinned exactly and annotated for Renovate's regex manager
(`renovate.json`): a `; renovate:` comment directly above the line ending
in `@<version>`.

**Adding a library** to the firmware:

```ini
[env:nodemcuv2]
lib_deps =
    ; renovate: datasource=github-tags depName=bblanchon/ArduinoJson
    bblanchon/ArduinoJson@7.2.1

[env:nodemcuv2_DEBUG]
lib_deps = ${env:nodemcuv2.lib_deps}
```

- `depName` is the GitHub `owner/repo` (the tags Renovate reads), not the
  registry name. Exact version, no `^` or `~`.
- If unit-tested code uses it, add it to `[env:unit_test]` `lib_deps` too.
- The Arduino IDE compile and the SIL build don't see `lib_deps`:
  - `.github/workflows/compile-sketch.yaml`: a `libraries:` input on the
    `arduino/compile-sketches` step (`- name: ArduinoJson` /
    `version: 7.2.1`). Renovate doesn't track that pin; add a regex manager
    to `renovate.json` or say so in the PR.
  - `test-sil/run.sh`: an include path in `cflags` (header-only libraries)
    or extra sources, fetched at a pinned version like the core.

**Bumping `espressif8266`** changes the ESP8266 Arduino core. Find the core
it brings with `pio pkg list -e nodemcuv2` (`framework-arduinoespressif8266
3.30102.0` is core `3.1.2`) and update all three pins together, comments
included:

1. `platform = espressif8266@…` in `platformio.ini`
2. `CORE_VERSION=` in `test-sil/run.sh` (may need
   `core-host-emulation.patch` updated)
3. `version:` in `.github/workflows/compile-sketch.yaml`

## Errors and warnings

- `InvalidProjectConfError: … No section: 'secret'`: `secrets.ini` is
  missing. `cp secrets.ini.template secrets.ini`; never read or print the
  real one.
- `Package Mirror: … usc1.contabostorage.com … 403 Forbidden … Looking for
  another mirror`: expected in cloud sessions, the next mirror works. It's
  only a failure if no "has been installed!" follows. Don't retry or work
  around the proxy.
- `pio: command not found`: the SessionStart hook puts its venv on `PATH`;
  elsewhere `pip install -r requirements.txt`.
- `undefined reference` in `pio test`: the source is missing from
  `build_src_filter`.

## The board

Never upload or open the serial monitor unless the user asks; the cloud
session has no board anyway. When asked, on the user's machine:
`pio run -t upload` and `pio device monitor` (115200 baud,
`esp8266_exception_decoder` decodes crash dumps in the DEBUG env).

## Before pushing

Run what CI runs for the kind of change, and read the output, not just the
exit code:

```sh
pio run -e nodemcuv2 -e nodemcuv2_DEBUG
pio test -e unit_test            # every unit_test* env
pio check -e nodemcuv2_DEBUG
test-sil/run.sh
pre-commit run --all-files
```

Plus clang-format 21 on changed C++ files.
