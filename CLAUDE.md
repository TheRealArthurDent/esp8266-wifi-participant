# esp8266-wifi-participant – WiFi participant for the ESP8266

A small firmware project that makes an ESP8266 (NodeMCU v2) a participant in a
WiFi network: it connects, keeps the connection, and informs registered
`WifiDependent`s when the connection is established or lost. The LEDs show the
state (ESP LED flashes while connecting, node LED blinks once connected). Built
with PlatformIO and the Arduino framework, and kept buildable in the Arduino
IDE.

## State of the repo

`main` is a skeleton: the WiFi controller, the `Metronome` loop helper and two
`DummyWifiDependent`s. Work that hasn't landed yet lives in old open PRs, all
conflicting with or lagging behind `main`:

- #29 "Main app" and #31 "Alerting" – an `App` class behind an `ArduinoCallable`
  wrapper, and an `Alert`/`MasterCaution` system.
- #80 "Introduce testing with Unity" – unit tests with Unity and a
  `platformio-test` workflow.

Known bug: `setup()` in `src/main.cpp` hands pointers to two stack-local
`DummyWifiDependent`s to `WifiConnection::init`, which keeps them after
`setup()` returns.

Onboarding Claude to this repo is tracked in #137 and its sub-issues. Check open
issues before changing an affected area, and reference them in commits.

## Layout

- `src/` – implementation (`main.cpp` holds `setup()`/`loop()`)
- `include/` – headers (`.hpp` for classes, `.h` for plain C-style helpers like
  `debug.h`, `led.h`)
- `lib/`, `test/` – PlatformIO's library and test folders (empty on `main`)
- `scripts/migrate_to_ino.sh` – copies `src/` and `include/` into a flat Arduino
  IDE sketch folder
- `docs/` – Doxygen config; the docs are published to GitHub Pages
- `.github/workflows/` – CI (see below)

## Hardware

Board: NodeMCU v2 (ESP8266), with two PlatformIO envs:

- `nodemcuv2_DEBUG` – **default**; debug build with `-D DEBUG`, serial logging
  at 115200 baud
- `nodemcuv2` – release build

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
pio test                              # unit tests (once #80 lands)
pre-commit run --all-files            # hygiene, JSON/YAML, Renovate, linters
clang-format --dry-run --Werror src/*.cpp include/*.h*   # see .clang-format
```

In Claude cloud sessions, `.claude/hooks/session-start.sh` does the setup above
(PlatformIO and pre-commit in a venv that is put on `PATH`, a dummy
`secrets.ini`, the pre-commit hooks and the PlatformIO packages), so every
command here runs there without manual steps. The pre-commit hooks don't use
Docker, because cloud sessions can't run containers.

CI on every PR: PlatformIO build and `pio check`, cpp-linter (clang-format +
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
  doesn't touch hardware (like `Metronome`) belongs under unit tests once Unity
  is in (#80).
- YAML files use the `.yaml` extension, not `.yml`.
- Everything in the repo is written in English (code, comments, docs, commit
  messages), even when the user chats in German.

## ESP8266 pitfalls

- Long blocking `delay()` loops starve the WiFi stack and can trigger watchdog
  resets; yield or keep loops short.
- Heap is small and fragments easily; avoid repeated `new`/`String` churn in
  `loop()`.
- Objects handed to callbacks or singletons must outlive them (see the known bug
  above).

## Working with the user

- Whenever a task would benefit from a skill you don't have, say so: which
  skill, what it would be for, and whether a ready-made one exists or it would
  have to be written.
