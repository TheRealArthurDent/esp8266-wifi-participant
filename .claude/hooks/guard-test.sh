#!/bin/bash
# Regression tests for guard.sh: bash .claude/hooks/guard-test.sh
# Each case feeds guard.sh a tool call and expects allow (0), block (2) or
# ask (A). Add a case for every false alarm and every gap you fix. Refs #137
# The cases are literal command lines, so $ and backticks stay unexpanded.
# shellcheck disable=SC2016
set -uo pipefail
guard="$(dirname "$0")/guard.sh"
failed=0

check() {
  local want=$1 json=$2 label=$3 out got
  out=$(bash "$guard" <<<"$json" 2>/dev/null)
  got=$?
  if [ "$got" = 0 ] && grep -q '"ask"' <<<"$out"; then got=A; fi
  if [ "$got" = "$want" ]; then
    echo "ok    $want  $label"
  else
    echo "FAIL  want $want, got $got  $label"
    failed=1
  fi
}
bash_cmd() { check "$1" "$(jq -n --arg c "$2" '{tool_name:"Bash",tool_input:{command:$c}}')" "Bash: $2"; }
write() { check "$1" "$(jq -n --arg p "$2" --arg t "$3" '{tool_name:"Write",tool_input:{file_path:$p,content:$t}}')" "Write $2: $3"; }
read_file() { check "$1" "$(jq -n --arg p "$2" '{tool_name:"Read",tool_input:{file_path:$p}}')" "Read: $2"; }

# Everyday commands pass, including earlier false alarms.
bash_cmd 0 'git log --oneline -6 && git show --stat HEAD^2 | head -30; grep -n "pre-commit\|venv" CLAUDE.md'
bash_cmd 0 'git log -n 5 --oneline'
bash_cmd 0 'git commit -m "build: run pre-commit -n checks"'
bash_cmd 0 'git commit -q -F - <<EOF'
bash_cmd 0 'git commit --amend --no-edit'
bash_cmd 0 'git push -u origin claude/project-thread-0n37sz'
bash_cmd 0 'git push -u origin claude/x && echo main'
bash_cmd 0 'grep -n commit file && git status'
bash_cmd 0 'grep -rn "pio run -v" .claude/skills/'
bash_cmd 0 'pio test -e unit_test -e unit_test_DEBUG'
bash_cmd 0 'pio run -e nodemcuv2'
bash_cmd 0 'cp secrets.ini.template secrets.ini'
bash_cmd 0 'git add .claude/hooks/guard.sh && git push -u origin claude/x 2>&1 | tail -1'
bash_cmd 0 'git diff .claude/settings.json > /tmp/d.txt'
bash_cmd 0 'cat .claude/settings.json'
bash_cmd 0 'git diff | head -5; git ls-files -s .claude/hooks/'
bash_cmd 0 'shellcheck .claude/hooks/guard.sh && echo ok > /tmp/x'
bash_cmd 0 'bash .claude/hooks/guard-test.sh | grep -v ^ok'
bash_cmd 0 '/opt/sc/bin/shellcheck .claude/hooks/guard.sh .claude/hooks/guard-test.sh'

# Secrets, also via globs and build outputs.
bash_cmd 2 'cat secrets.ini'
bash_cmd 2 'head secrets*'
bash_cmd 2 'cat secret?.ini'
bash_cmd 2 'pio run -v'
bash_cmd 2 'platformio run -v'
bash_cmd 2 'pio project config'
bash_cmd 2 'pio run -t compiledb'
bash_cmd 2 'grep -r SECRET_WIFI_PASS .pio/build'
bash_cmd 2 'strings .pio/build/nodemcuv2/firmware.elf'
bash_cmd 2 'jq . .pio/build/nodemcuv2_DEBUG/idedata.json'
read_file 2 /repo/secrets.ini
read_file 2 /repo/.pio/build/nodemcuv2_DEBUG/idedata.json
read_file 0 /repo/src/main.cpp

# main, force pushes and skipped hooks.
bash_cmd 2 'git push origin main'
bash_cmd 2 'git push origin HEAD:main'
bash_cmd 2 'git push --force origin claude/x'
bash_cmd 2 'git push --force-with-lease'
bash_cmd 2 'git push -f'
bash_cmd 2 'git push origin +claude/x'
bash_cmd 2 'git push --no-verify'
bash_cmd 2 'git commit --no-verify -m x'
bash_cmd 2 'git commit -n -m x'
bash_cmd 2 'git commit -nm "x"'
bash_cmd 2 'git commit -anm x'
bash_cmd 2 'git add . && git commit -n -m x'
bash_cmd 2 'SKIP=gitleaks git commit -m x'
bash_cmd 2 'git -c core.hooksPath=/dev/null commit -m x'
bash_cmd 2 'pre-commit uninstall'
bash_cmd 2 'chmod -x .git/hooks/pre-commit'

# Hardware asks first, whichever name PlatformIO is called by.
bash_cmd A 'pio run -e nodemcuv2 -t upload'
bash_cmd A 'pio run --target=upload'
bash_cmd A 'pio device monitor'
bash_cmd A 'platformio run -t upload'
bash_cmd A 'platformio device monitor'
bash_cmd A 'python3 -m platformio run -t upload'
bash_cmd A 'esptool.py --port /dev/ttyUSB0 erase_flash'

bash_cmd 2 'docker run x'

# The guardrails themselves.
bash_cmd 2 'echo x > .claude/settings.json'
bash_cmd 2 'echo x >> ".claude/hooks/guard.sh"'
bash_cmd 2 'echo x | tee .claude/settings.json'
bash_cmd 2 'sed -i s/a/b/ .claude/hooks/guard.sh'
bash_cmd 2 'rm .claude/hooks/guard.sh'
bash_cmd 2 'git status && chmod -x .claude/hooks/guard.sh'
bash_cmd 2 'cp /tmp/x .claude/settings.json'
bash_cmd 2 "python3 -c \"open('.claude/settings.json','w').write('{}')\""
bash_cmd 2 'perl -pi -e s/guard/x/ .claude/settings.json'
bash_cmd 2 'git checkout HEAD~5 -- .claude/hooks/guard.sh'
bash_cmd 2 'echo {} | python3 -c "import sys;open(\".claude/settings.local.json\",\"w\").write(sys.stdin.read())"'
write A /repo/.claude/settings.local.json '{"disableAllHooks": true}'
write A /repo/.claude/settings.json '{}'
write A /repo/.claude/hooks/guard.sh 'exit 0'
write A /home/user/.claude/settings.json '{"disableAllHooks": true}'
write 2 /repo/secrets.ini 'x'

# Suppressions: targeted only, and only checked in code.
write 2 /repo/src/a.cpp 'int x; // NOLINT'
write 2 /repo/include/a.hpp '// NOLINTNEXTLINE'
write 2 /repo/src/a.cpp '// cppcheck-suppress'
write 0 /repo/src/a.cpp 'int x; // NOLINT(readability-x)'
write 0 /repo/src/a.cpp '// NOLINTNEXTLINE(cert-x)'
write 0 /repo/CLAUDE.md 'untargeted `NOLINT`/`cppcheck-suppress`'

# Weakening CI asks first.
write A /repo/.github/workflows/a.yaml 'continue-on-error: true'
write A /repo/platformio.ini 'test_ignore = *'
write 0 /repo/.github/workflows/a.yaml 'runs-on: ubuntu-latest'

exit "$failed"
