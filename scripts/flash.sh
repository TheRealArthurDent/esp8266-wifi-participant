#!/usr/bin/env bash
# Builds and flashes the board with the real hostname and WiFi credentials,
# which live outside the repo so that Claude Code's sandbox can keep them out
# of reach. The repo's secrets.ini only needs dummy values for building and
# testing. Run it yourself; Claude only flashes when you ask.
#
#   scripts/flash.sh                      # flash the default env (nodemcuv2_DEBUG)
#   scripts/flash.sh -e nodemcuv2         # the release build instead
#   scripts/flash.sh -t monitor           # flash, then open the serial monitor
#
# Arguments go to `pio run -t upload`. The real values are read from
# ~/.config/esp8266-wifi-participant/secrets.ini (same format as
# secrets.ini.template), and the build lands in
# ~/.cache/esp8266-wifi-participant/build instead of .pio/build, because the
# build outputs contain the values too.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

REAL_CONFIG="${HOME}/.config/esp8266-wifi-participant/secrets.ini"
BUILD_DIR="${HOME}/.cache/esp8266-wifi-participant/build"
VENV="${VENV:-${HOME}/.venvs/esp8266-wifi-participant}"

if [ ! -f "${REAL_CONFIG}" ]; then
  cat >&2 <<EOF
${REAL_CONFIG} is missing. Create it with your real values:

  mkdir -p "$(dirname "${REAL_CONFIG}")"
  cp secrets.ini.template "${REAL_CONFIG}"
  chmod 600 "${REAL_CONFIG}"

then edit it. See README.md, "Keeping the WiFi credentials away from Claude".
EOF
  exit 1
fi

PIO="$(command -v pio || echo "${VENV}/bin/pio")"

ESP8266_REAL_CONFIG="${REAL_CONFIG}" PLATFORMIO_BUILD_DIR="${BUILD_DIR}" \
  exec "${PIO}" run -t upload "$@"
