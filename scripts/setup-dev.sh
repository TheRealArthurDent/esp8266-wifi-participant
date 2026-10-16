#!/usr/bin/env bash
# Sets up a local development checkout: the pinned Python tools from
# requirements.txt (PlatformIO, pre-commit) in a venv outside the repo, a
# secrets.ini from the template, the pre-commit Git hook with its hook
# environments, and the PlatformIO packages all envs need. Safe to run again at any time, e.g. after requirements.txt or
# .pre-commit-config.yaml changed.
#
#   scripts/setup-dev.sh
#
# VENV overrides the venv location (default: the one the Claude cloud session
# uses, ~/.venvs/esp8266-wifi-participant).
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

VENV="${VENV:-${HOME}/.venvs/esp8266-wifi-participant}"

if [ ! -x "${VENV}/bin/python" ]; then
  echo "Creating venv in ${VENV}"
  if ! python3 -m venv "${VENV}"; then
    echo "python3 -m venv failed; on Debian/Ubuntu install python3-venv first." >&2
    exit 1
  fi
fi

echo "Installing requirements.txt into ${VENV}"
"${VENV}/bin/pip" install --quiet --disable-pip-version-check -r requirements.txt

# The template's dummy values build and test fine. The real hostname and WiFi
# credentials go outside the repo, where scripts/flash.sh reads them. An
# existing secrets.ini is never touched.
if [ ! -f secrets.ini ]; then
  cp secrets.ini.template secrets.ini
  echo "Created secrets.ini from the template (dummy values)."
fi
if [ ! -f "${HOME}/.config/esp8266-wifi-participant/secrets.ini" ]; then
  echo "To flash a board, put your real values in" \
    "${HOME}/.config/esp8266-wifi-participant/secrets.ini (see README.md)."
fi

"${VENV}/bin/pre-commit" install
"${VENV}/bin/pre-commit" install-hooks

# Platform, framework and toolchain for all envs (unit tests included), plus
# the pio check tools, so that builds, tests and checks also run offline,
# e.g. inside Claude Code's sandbox, which blocks the PlatformIO registry.
"${VENV}/bin/pio" pkg install -e nodemcuv2 -e nodemcuv2_DEBUG -e unit_test -e unit_test_DEBUG
if [ ! -d "${HOME}/.platformio/packages/tool-cppcheck" ] || [ ! -d "${HOME}/.platformio/packages/tool-clangtidy" ]; then
  "${VENV}/bin/pio" check -e nodemcuv2_DEBUG --silent
fi

case ":${PATH}:" in
  *":${VENV}/bin:"*) ;;
  *)
    echo
    echo "pio and pre-commit are not on your PATH yet. Add them permanently with"
    echo "  fish:      fish_add_path ${VENV}/bin"
    echo "  bash/zsh:  echo 'export PATH=\"${VENV}/bin:\$PATH\"' >> ~/.bashrc  # or ~/.zshrc"
    echo "or activate the venv per shell with: source ${VENV}/bin/activate(.fish)"
    ;;
esac
