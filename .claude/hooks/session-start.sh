#!/bin/bash
# Prepares a Claude Code cloud session to build and check this project like CI
# does: PlatformIO, pre-commit, a dummy secrets.ini and a warm package cache.
# Refs #140
set -euo pipefail

if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
  exit 0
fi

cd "${CLAUDE_PROJECT_DIR:-$(git rev-parse --show-toplevel)}"

# Pinned Python tooling (PlatformIO, pre-commit) in a venv outside the repo.
venv="${HOME}/.venvs/esp8266-wifi-participant"
if [ ! -x "${venv}/bin/python" ]; then
  python3 -m venv "${venv}"
fi
"${venv}/bin/pip" install --quiet --disable-pip-version-check -r requirements.txt
export PATH="${venv}/bin:${PATH}"
if [ -n "${CLAUDE_ENV_FILE:-}" ]; then
  echo "export PATH=\"${venv}/bin:\${PATH}\"" >> "${CLAUDE_ENV_FILE}"
fi

# Dummy values only, so the build flags resolve. Never real credentials.
if [ ! -f secrets.ini ]; then
  cp secrets.ini.template secrets.ini
fi

pre-commit install
pre-commit install-hooks

# Platform, framework and toolchain for all envs (unit tests included), plus
# the pio check tools.
pio pkg install -e nodemcuv2 -e nodemcuv2_DEBUG -e unit_test
if [ ! -d "${HOME}/.platformio/packages/tool-cppcheck" ] || [ ! -d "${HOME}/.platformio/packages/tool-clangtidy" ]; then
  pio check -e nodemcuv2_DEBUG --silent
fi
