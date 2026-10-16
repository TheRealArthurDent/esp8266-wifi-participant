#!/bin/bash
# PreToolUse guard for risky actions in esp8266-wifi-participant.
# Exit 2 blocks the tool call and shows stderr to Claude. Refs #137
#
# It matches command text, so it catches mistakes, not someone set on getting
# around it (a shell function or a script does). What must be impossible lives
# server-side: the rulesets in .github/rulesets/ and the required CI checks.
# Test changes with .claude/hooks/guard-test.sh.
set -uo pipefail
# Without jq nothing below can be checked, so fail closed.
if ! command -v jq >/dev/null 2>&1; then
  echo "Blocked by .claude/hooks/guard.sh: jq is missing, install it first." >&2
  exit 2
fi
input=$(cat)
tool=$(jq -r '.tool_name' <<<"$input")
cmd=$(jq -r '.tool_input.command // empty' <<<"$input")
path=$(jq -r '.tool_input.file_path // .tool_input.notebook_path // .tool_input.path // empty' <<<"$input")
text=$(jq -r '[.tool_input.content, .tool_input.new_string, (.tool_input.edits // [] | .[].new_string)] | map(select(. != null)) | join("\n")' <<<"$input")

block() { echo "Blocked by .claude/hooks/guard.sh: $1" >&2; exit 2; }
ask() { jq -n --arg r "$1" '{hookSpecificOutput:{hookEventName:"PreToolUse",permissionDecision:"ask",permissionDecisionReason:$r}}'; exit 0; }
# One command per line: split at &&, ||, ; and |.
split() { awk '{ gsub(/&&|\|\||;|\|/, "\n") } 1' <<<"$1"; }

# The build passes the secrets.ini values as -D flags, so they end up in the
# build outputs too (idedata.json, compile_commands.json, the firmware).
BUILD_SECRETS='\.pio/build|idedata\.json|compile_commands\.json|firmware\.(elf|bin)'
# The real values live outside the repo; only scripts/flash.sh reads them.
REAL_SECRETS='\.(config|cache)/esp8266-wifi-participant'
GUARDRAILS='\.claude/(settings(\.local)?\.json|hooks/)|\.git/hooks/'

case "$tool" in
  Bash)
    # Quoted strings (commit messages, grep patterns) don't count as commands.
    unquoted=$(sed -E "s/\"[^\"]*\"//g; s/'[^']*'//g" <<<"$cmd")

    # Secrets: no reading, printing or staging secrets.ini (globs included),
    # no verbose builds, no reading build outputs that embed the values.
    grep -Eq 'secret[s*?[]' <<<"$cmd" && ! grep -Eq '^cp secrets\.ini\.template secrets\.ini$' <<<"$cmd" \
      && block "secrets.ini holds real WiFi credentials; never read, print, edit or stage it."
    while IFS= read -r seg; do
      grep -Eq "$BUILD_SECRETS" <<<"$seg" \
        && grep -Eq '\b(cat|less|more|head|tail|grep|rg|strings|jq|xxd|od|python3?|cp|base64)\b' <<<"$seg" \
        && block "build outputs contain the WiFi credentials as -D flags."
      grep -Eq "$REAL_SECRETS" <<<"$seg" \
        && grep -Eq '\b(cat|less|more|head|tail|grep|rg|strings|jq|xxd|od|python3?|cp|mv|base64|ls|find|sed|awk)\b' <<<"$seg" \
        && block "the real WiFi credentials and their builds are off limits."
    done < <(split "$cmd")
    grep -Eq '\bESP8266_REAL_CONFIG=' <<<"$unquoted" \
      && block "only scripts/flash.sh, run by the user, builds with the real WiFi credentials."
    pio_cmds=$(split "$unquoted" | grep -E '^\s*(\S*/)?(pio|platformio)\b|python3? -m platformio' || true)
    grep -Eq '\b(run|test|check)\b.*(\s-v\b|--verbose)|\bproject config\b|(-t|--target)[ =]?compiledb' <<<"$pio_cmds" \
      && block "verbose PlatformIO output prints SECRET_WIFI_PASS."

    # main is protected: no direct push, no force push, no skipping hooks.
    git_cmds=$(split "$unquoted" | grep -E '^\s*([A-Za-z_]+=\S*\s+)*git\b' || true)
    grep -Eq '\bpush\b.*(\bmain\b|\bmaster\b|--force\b|\s-f\b|\s\+)' <<<"$git_cmds" \
      && block "push only feature branches, never main, never force."
    grep -Eq -- '--no-verify|\bcommit\b.*\s-[A-Za-z]*n[A-Za-z]*(\s|$)' <<<"$git_cmds" \
      && block "pre-commit hooks must not be skipped."
    grep -Eq '(^|\s)SKIP=|PRE_COMMIT_ALLOW_NO_CONFIG|core\.hooksPath|pre-commit uninstall' <<<"$unquoted" \
      && block "pre-commit hooks must not be skipped."

    # Hardware: flashing or serial only on the user's explicit word.
    grep -Eq '(-t|--target)[ =]?(upload|uploadfs|monitor|erase)|\b(device|remote)\b' <<<"$pio_cmds" \
      && ask "Flashing or serial access on a real board: only when the user asked for it."
    split "$unquoted" | grep -Eq '^\s*([A-Za-z_]+=\S*\s+)*((ba)?sh\s+)?\S*flash\.sh\b' \
      && ask "Flashing or serial access on a real board: only when the user asked for it."
    grep -Eq 'esptool|/dev/tty(USB|ACM)' <<<"$cmd" \
      && ask "Flashing or serial access on a real board: only when the user asked for it."

    # Containers are not allowed in cloud sessions.
    grep -Eq '\b(docker|podman)\b' <<<"$unquoted" && block "no containers here (see team memory cloud-session-no-docker)."

    # The guardrails themselves: a command that names one may only read it
    # (or run the guard's own tests), never write or redirect into it.
    while IFS= read -r seg; do
      grep -Eq "$GUARDRAILS" <<<"$seg" || continue
      if grep -Eq '^\s*(\S*/)?(cat|less|head|tail|grep|rg|diff|ls|stat|wc|shellcheck|bash -n|jq|git (add|diff|log|show|status|blame|ls-files))\b|^\s*(bash )?\S*\.claude/hooks/guard-test\.sh\s*$' <<<"$seg" \
        && ! grep -Eq ">>?\s*[\"']?[^ ]*($GUARDRAILS)" <<<"$seg"; then
        continue
      fi
      block "guardrail files change only through a reviewed PR the user asked for."
    done < <(split "$cmd")
    ;;
  Read|Grep)
    grep -Eq "secrets\.ini$|$BUILD_SECRETS|$REAL_SECRETS" <<<"$path" \
      && block "secrets.ini and build outputs contain the WiFi credentials."
    ;;
  Edit|Write|MultiEdit|NotebookEdit)
    case "$path" in
      */secrets.ini) block "secrets.ini is off limits." ;;
      */.claude/settings.json|*/.claude/settings.local.json|*/.claude/hooks/*|*/.git/hooks/*)
        ask "Changes a guardrail (Claude settings or a hook)." ;;
    esac
    # Suppressions only targeted, and only in code: NOLINT(<check>),
    # cppcheck-suppress <id>. Docs may mention the bare keyword.
    case "$path" in
      *.c|*.cpp|*.h|*.hpp|*.ino)
        grep -Eq 'NOLINT(NEXTLINE|BEGIN)?([^(A-Za-z]|$)' <<<"$text" && block "bare NOLINT; name the check: NOLINT(<check>)."
        grep -Eq 'cppcheck-suppress\s*$' <<<"$text" && block "cppcheck-suppress needs an id."
        ;;
      */.github/workflows/*|*/.pre-commit-config.yaml|*/.clang-tidy|*/.clang-format|*/platformio.ini)
        grep -Eq 'continue-on-error:\s*true|if:\s*(false|\$\{\{\s*false)|test_ignore|check_skip|--no-verify' <<<"$text" \
          && ask "This weakens CI or a check; confirm the user asked for it."
        ;;
    esac
    ;;
esac
exit 0
