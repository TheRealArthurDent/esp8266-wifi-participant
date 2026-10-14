#!/bin/bash
# Scenarios are called by name, which shellcheck can't follow.
# shellcheck disable=SC2329

# Software-in-the-loop (SIL) tests: builds the firmware against the ESP8266
# Arduino core's host emulation, runs it as a native Linux program and checks
# its serial output.
#
# Usage: test-sil/run.sh [scenario...]   (default: all scenarios)
#
# The core is fetched from GitHub into .pio/sil on the first run.
# See test-sil/README.md.
# Refs #143
set -euo pipefail

# The core PlatformIO builds with (framework-arduinoespressif8266 3.30102.0).
# A new version may need core-host-emulation.patch updated.
# renovate: datasource=github-tags depName=esp8266/Arduino
CORE_VERSION=3.1.2

repo="$(cd "$(dirname "$0")/.." && pwd)"
work="${repo}/.pio/sil"
core="${work}/arduino-${CORE_VERSION}"
sketch="${work}/sketch/firmware/firmware"
log="${work}/logs"
binary="${core}/tests/host/bin/firmware/firmware"

fetch_core()
{
  if [ -f "${core}/.patched" ]; then
    return
  fi
  rm -rf "${core}"
  echo "Fetching ESP8266 Arduino core ${CORE_VERSION}..."
  git -c advice.detachedHead=false clone --quiet --depth 1 --branch "${CORE_VERSION}" https://github.com/esp8266/Arduino.git "${core}"
  # Only the submodules the emulation compiles.
  git -C "${core}" submodule --quiet update --init --depth 1 \
    libraries/LittleFS/lib/littlefs libraries/ESP8266SdFat libraries/SoftwareSerial \
    tools/sdk/ssl/bearssl tools/sdk/uzlib
  git -C "${core}" apply "${repo}/test-sil/core-host-emulation.patch"
  touch "${core}/.patched"
}

build()
{
  mkdir -p "$(dirname "${sketch}")"
  # The emulation has no Arduino preprocessing; the sketch just includes main.cpp.
  printf '#include "%s"\n#include "%s"\n' "${repo}/src/main.cpp" "${repo}/test-sil/wifi-events.cpp" > "${sketch}.ino"

  local sources=()
  for f in "${repo}"/src/*.cpp; do
    [ "$(basename "${f}")" = main.cpp ] || sources+=("${f}")
  done

  # glibc 2.38 and later ship strlcpy/strlcat, the emulation's own copies clash.
  local cflags="-I${repo}/include -DDEBUG -DMONITOR_SPEED=115200"
  cflags+=" -DSECRET_HOSTNAME=\\\"emulated-host\\\" -DSECRET_WIFI_SSID=\\\"emulated-ssid\\\" -DSECRET_WIFI_PASS=\\\"emulated-pass\\\""
  cflags+=" -fsanitize=address -fno-omit-frame-pointer"
  if printf '#include <cstring>\nint main(){char a[4]{}; return strlcpy(a, "x", sizeof(a)) + strlcat(a, "y", sizeof(a));}\n' |
    g++ -x c++ - -o /dev/null 2>/dev/null; then
    cflags+=" -DHAVE_STRLCAT -DHAVE_STRLCPY"
  fi

  echo "Building firmware for the host..."
  mkdir -p "${log}"
  if ! make -C "${core}/tests/host" -j"$(nproc)" FORCE32=0 R=noexec \
    USERCFLAGS="${cflags}" USERLDFLAGS="-fsanitize=address" \
    USERCXXSOURCES="${sources[*]}" "${sketch}" > "${log}/build.log" 2>&1; then
    tail -n 30 "${log}/build.log"
    echo "Build failed, see ${log}/build.log"
    exit 1
  fi
}

# run_firmware <name> <seconds> [WIFI_EVENTS]
# Runs the firmware for the given time; output goes to ${log}/<name>.log.
run_firmware()
{
  local name="$1" seconds="$2" events="${3:-}"
  local status=0
  mkdir -p "${work}/fs"
  # Stack frames that outlive their function are caught by ASan.
  WIFI_EVENTS="${events}" ASAN_OPTIONS="detect_stack_use_after_return=1" \
    timeout --signal=INT "${seconds}" "${binary}" -P "${work}/fs" < /dev/null > "${log}/${name}.log" 2>&1 || status=$?
  echo "${status}" > "${log}/${name}.status"
}

failures=0
current=""

pass() { echo "  ok    $1"; }
fail()
{
  echo "  FAIL  $1"
  failures=$((failures + 1))
}

# expect_line <text> [count]: the output contains <text> exactly [count] times
# (default: at least once).
expect_line()
{
  local text="$1" count="${2:-}"
  local found
  found="$(grep -cF -- "${text}" "${log}/${current}.log" || true)"
  if [ -z "${count}" ] && [ "${found}" -gt 0 ]; then
    pass "'${text}'"
  elif [ -n "${count}" ] && [ "${found}" -eq "${count}" ]; then
    pass "'${text}' x${count}"
  else
    fail "'${text}' x${count:-1+}, found ${found}"
  fi
}

# expect_running: the firmware ran until the timeout, without crashing.
expect_running()
{
  local status
  status="$(cat "${log}/${current}.status")"
  if [ "${status}" = 124 ]; then
    pass "still running at timeout"
  else
    fail "exited with status ${status} before the timeout"
    grep -m 3 -E 'ERROR: AddressSanitizer|SUMMARY' "${log}/${current}.log" | sed 's/^/        /' || true
  fi
}

scenario_boots()
{
  run_firmware boots 6
  expect_running
  expect_line "Debugging initialized."
  expect_line "Connecting to emulated-ssid"
  expect_line "Loop ended at:"
}

scenario_wifi_events()
{
  run_firmware wifi_events 4 "got_ip@1000,disconnected@2000,got_ip@3000"
  expect_running
  expect_line "Connected to WiFi emulated-ssid" 2
  # localIP() is the host's own address in the emulation.
  expect_line "IP address: " 2
  expect_line "Disconnected from WiFi" 1
  expect_line "DummyWifiDependent connected!" 4
  expect_line "DummyWifiDependent disconnected!" 2
}

all_scenarios=(boots wifi_events)

# Scenarios that fail on main because of a known bug. They must keep failing
# until it's fixed; then remove them here.
known_failures=()

fetch_core
build

scenarios=("$@")
[ ${#scenarios[@]} -gt 0 ] || scenarios=("${all_scenarios[@]}")

unexpected=0
for s in "${scenarios[@]}"; do
  echo "Scenario ${s}:"
  current="${s}"
  failures=0
  "scenario_${s}"
  known=no
  for k in "${known_failures[@]}"; do
    [ "${k}" = "${s}" ] && known=yes
  done
  if [ "${failures}" -gt 0 ] && [ "${known}" = yes ]; then
    echo "  -> known failure (log: ${log}/${s}.log)"
  elif [ "${failures}" -gt 0 ]; then
    echo "  -> FAILED (log: ${log}/${s}.log)"
    unexpected=$((unexpected + 1))
  elif [ "${known}" = yes ]; then
    echo "  -> passed, but is listed as a known failure; remove it from known_failures"
    unexpected=$((unexpected + 1))
  else
    echo "  -> passed"
  fi
done

exit "$((unexpected > 0))"
