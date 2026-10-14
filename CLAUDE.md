# esp8266-wifi-participant – WiFi participant for the ESP8266

A small firmware project that makes an ESP8266 (NodeMCU v2) a participant in a
WiFi network: it connects, keeps the connection, and informs registered
`WifiDependent`s when the connection is established or lost. The LEDs show the
state (ESP LED flashes while connecting, node LED blinks once connected). Built
with PlatformIO and the Arduino framework, and kept buildable in the Arduino
IDE.

## State of the repo

`main` is a skeleton: the WiFi controller, the `Metronome` loop helper (with
unit tests) and two `DummyWifiDependent`s. Work that hasn't landed yet lives in
old open PRs, all conflicting with or lagging behind `main`:

- #29 "Main app" and #31 "Alerting" – an `App` class behind an `ArduinoCallable`
  wrapper, and an `Alert`/`MasterCaution` system.

Onboarding Claude to this repo is tracked in #137 and its sub-issues. Check open
issues before changing an affected area, and reference them in commits.

## Layout

- `src/` – implementation (`main.cpp` holds `setup()`/`loop()`)
- `include/` – headers (`.hpp` for classes, `.h` for plain C-style helpers like
  `debug.h`, `led.h`)
- `lib/` – PlatformIO's library folder (empty on `main`)
- `test/` – unit tests, one `test_<name>/` folder per suite, plus shared test
  helpers such as `fakeit-unity.hpp` and a `.clang-tidy` for test code
- `test-sil/` – software-in-the-loop (SIL) tests: the real firmware and ESP8266
  core libraries built as a Linux program, see `test-sil/README.md`
- `scripts/migrate_to_ino.sh` – copies `src/` and `include/` into a flat Arduino
  IDE sketch folder
- `docs/` – Doxygen config; the docs are published to GitHub Pages
- `.github/workflows/` – CI (see below)

## Hardware

Board: NodeMCU v2 (ESP8266), with two PlatformIO envs:

- `nodemcuv2_DEBUG` – **default**; debug build with `-D DEBUG`, serial logging
  at 115200 baud
- `nodemcuv2` – release build

A third env, `unit_test`, builds the unit tests for the host (`platform =
native`), not for the board.

Platform: `espressif8266`, framework: `arduino`. Never flash a board unless the
user explicitly asks.

## Secrets

`secrets.ini` holds the hostname, WiFi SSID and WiFi password. `platformio.ini`
includes it and passes the values as `SECRET_HOSTNAME`, `SECRET_WIFI_SSID` and
`SECRET_WIFI_PASS` build flags. Create it from `secrets.ini.template` (CI does
exactly that, with dummy values). `secrets.ini` is gitignored: never commit it,
never print or read its values, and never put real credentials anywhere else in
the repo.

## Commands

```sh
cp secrets.ini.template secrets.ini   # once; dummy values build fine
pip install -r requirements.txt       # PlatformIO and pre-commit, pinned
pio run                               # build the default env (nodemcuv2_DEBUG)
pio run -e nodemcuv2                  # build the release env
pio check -e nodemcuv2_DEBUG          # static analysis: cppcheck + clang-tidy
pio test -e unit_test                 # unit tests on the host
test-sil/run.sh                      # SIL tests (firmware in host emulation)
pre-commit run --all-files            # hygiene, JSON/YAML, Renovate, linters
clang-format --dry-run --Werror src/*.cpp include/*.h*   # see .clang-format
```

In Claude cloud sessions, `.claude/hooks/session-start.sh` does the setup above
(PlatformIO and pre-commit in a venv that is put on `PATH`, a dummy
`secrets.ini`, the pre-commit hooks and the PlatformIO packages), so every
command here runs there without manual steps. The pre-commit hooks don't use
Docker, because cloud sessions can't run containers.

CI on every PR: PlatformIO build, `pio check` and `pio test -e unit_test`,
software-in-the-loop tests (`test-sil/run.sh`), cpp-linter (clang-format +
clang-tidy, reports only, doesn't fail the build), Arduino IDE compile
(`compile-sketch`) and `arduino-lint` (`check-arduino`), CodeQL, codespell,
markdownlint, reviewdog.

## Conventions

- Commit messages: [Conventional Commits](https://www.conventionalcommits.org/)
  (`feat:`, `fix:`, `docs:`, `style:`, `refactor:`, `perf:`, `test:`, `build:`,
  `ci:`, `chore:`, `revert:`), referencing issues (`Refs #138`).
- Work on feature branches; `main` is the default branch and the
  `no-commit-to-branch` pre-commit hook blocks commits to it.
- C++ formatting follows `.clang-format` (Allman braces, 2-space indent, no
  column limit), static checks follow `.clang-tidy`. Suppress a finding only
  with a targeted `NOLINT(<check>)` or `cppcheck-suppress <id>` comment, as the
  existing code does.
- Keep the code buildable in the Arduino IDE: `migrate_to_ino.sh` flattens
  `src/` and `include/` into one folder and renames `.cpp` to `.ino`, so no
  subfolders in `src/`/`include/`, no duplicate file names across them, and
  `main.cpp` stays the sketch entry point.
- Document public classes and methods with Doxygen comments (`\brief`, `\param`,
  `\c`), as the existing headers do.
- Dependencies (GitHub Actions, Python packages, pre-commit hooks) are pinned
  and updated by Renovate; keep them pinned. Leave Renovate PRs alone unless
  asked (automerge is being set up in #141).
- Work test-driven where it's feasible, and say why when it isn't. Logic that
  doesn't touch hardware (like `Metronome`) belongs under unit tests.

## Unit tests

- Tests use Unity and run on the host in the `unit_test` env. Plain `pio test`
  runs nothing, because the board envs ignore all tests; always pass
  `-e unit_test`.
- Arduino functions (`millis()`, `delay()`, `digitalWrite()`, …) come from
  [ArduinoFake](https://github.com/FabioBatSilva/ArduinoFake), which mocks them
  with fakeit: stub with `When(Method(ArduinoFake(), millis)).Return(…)`, check
  with `Verify(Method(ArduinoFake(), delay).Using(…)).Once()`. Call
  `ArduinoFakeReset()` in `setUp()` and `installFakeitUnityAdapter()` (from
  `test/fakeit-unity.hpp`) before `UNITY_BEGIN()`, so a failed `Verify` fails
  its test instead of aborting the program.
- Only code listed in `build_src_filter` of `[env:unit_test]` is built for the
  tests. Add a source file there when you test it; code that needs the
  ESP8266 SDK (like `WifiConnection`) can't be built on the host.
- `main()` returns `UNITY_END()`, so failures fail the process.
- YAML files use the `.yaml` extension, not `.yml`.
- Everything in the repo is written in English (code, comments, docs, commit
  messages), even when the user chats in German.

## Software-in-the-loop (SIL) tests

- `test-sil/run.sh` builds `src/` with the ESP8266 core's host emulation and
  runs scenarios (shell functions `scenario_<name>`) that check the serial
  output. Use it for what unit tests can't reach: `setup()`/`loop()` wiring,
  object lifetimes (AddressSanitizer is on) and WiFi event callbacks
  (`WIFI_EVENTS="got_ip@1000,disconnected@2000"`).
- A scenario that fails because of a known bug goes in `known_failures` with
  its issue; the fix removes it from there.
- The core version is pinned in `run.sh` and must match the one PlatformIO
  uses.

## ESP8266 pitfalls

- Long blocking `delay()` loops starve the WiFi stack and can trigger watchdog
  resets; yield or keep loops short.
- Heap is small and fragments easily; avoid repeated `new`/`String` churn in
  `loop()`.
- Objects handed to callbacks or singletons must outlive them, like the
  static `DummyWifiDependent`s in `main.cpp` that `WifiConnection` keeps.

## Working with the user

- Whenever a task would benefit from a skill you don't have, say so: which
  skill, what it would be for, and whether a ready-made one exists or it would
  have to be written.
