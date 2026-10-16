#!/bin/bash
# PreToolUse guard for risky actions in esp8266-wifi-participant.
# Exit 2 blocks the tool call and shows stderr to Claude. Refs #137
set -uo pipefail
input=$(cat)
tool=$(jq -r '.tool_name' <<<"$input")
cmd=$(jq -r '.tool_input.command // empty' <<<"$input")
path=$(jq -r '.tool_input.file_path // empty' <<<"$input")
text=$(jq -r '[.tool_input.content, .tool_input.new_string, (.tool_input.edits // [] | .[].new_string)] | map(select(. != null)) | join("\n")' <<<"$input")

block() { echo "Blocked by .claude/hooks/guard.sh: $1" >&2; exit 2; }
ask() { jq -n --arg r "$1" '{hookSpecificOutput:{hookEventName:"PreToolUse",permissionDecision:"ask",permissionDecisionReason:$r}}'; exit 0; }

case "$tool" in
  Bash)
    # Secrets: no reading, printing or staging secrets.ini, no verbose builds that echo the build flags.
    grep -Eq 'secrets\.ini' <<<"$cmd" && ! grep -Eq '^cp secrets\.ini\.template secrets\.ini$' <<<"$cmd" \
      && block "secrets.ini holds real WiFi credentials; never read, print, edit or stage it."
    grep -Eq '\bpio (run|test|check)\b.*(\s-v\b|--verbose)|\bpio project config\b' <<<"$cmd" \
      && block "verbose PlatformIO output prints SECRET_WIFI_PASS."
    # main is protected: no direct push, no force push, no skipping hooks.
    # Look at each git command on its own, without quoted strings (commit
    # messages, grep patterns), so `git log && grep -n "pre-commit"` passes.
    unquoted=$(sed -E "s/\"[^\"]*\"//g; s/'[^']*'//g" <<<"$cmd")
    git_cmds=$(awk '{ gsub(/&&|\|\||;|\|/, "\n") } 1' <<<"$unquoted" | grep -E '^\s*([A-Za-z_]+=\S*\s+)*git\b' || true)
    grep -Eq '\bpush\b.*(\bmain\b|\bmaster\b|--force\b|\s-f\b|\s\+)' <<<"$git_cmds" \
      && block "push only feature branches, never main, never force."
    grep -Eq -- '--no-verify|\bcommit\b.*\s-[A-Za-z]*n[A-Za-z]*(\s|$)' <<<"$git_cmds" \
      && block "pre-commit hooks must not be skipped."
    grep -Eq '(^|\s)SKIP=|PRE_COMMIT_ALLOW_NO_CONFIG|core\.hooksPath' <<<"$unquoted" \
      && block "pre-commit hooks must not be skipped."
    # Hardware: flashing or serial only on the user's explicit word.
    grep -Eq '\bpio\b.*(-t|--target)[ =]?(upload|uploadfs|monitor|erase)|\bpio (device|remote)\b|esptool|/dev/tty(USB|ACM)' <<<"$cmd" \
      && ask "Flashing or serial access on a real board: only when the user asked for it."
    # Containers are not allowed in cloud sessions.
    grep -Eq '\b(docker|podman)\b' <<<"$cmd" && block "no containers here (see team memory cloud-session-no-docker)."
    # The guardrails themselves.
    # Per command again: a write tool next to a guardrail path, or a redirect
    # into one. Quotes stay, so a quoted redirect target is still seen.
    while IFS= read -r seg; do
      grep -Eq '\.claude/(settings\.json|hooks/)' <<<"$seg" || continue
      grep -Eq "\b(sed|tee|mv|rm|cp|chmod|truncate)\b|>>?\s*[\"']?[^ ]*\.claude/(settings\.json|hooks/)" <<<"$seg" \
        && block "guardrail files change only through a reviewed PR the user asked for."
    done < <(awk '{ gsub(/&&|\|\||;|\|/, "\n") } 1' <<<"$cmd")
    ;;
  Edit|Write|MultiEdit|NotebookEdit)
    case "$path" in
      */secrets.ini) block "secrets.ini is off limits." ;;
      */.claude/settings.json|*/.claude/hooks/*) ask "Changes a guardrail (.claude/settings.json or a hook)." ;;
    esac
    # Suppressions only targeted: NOLINT(<check>), cppcheck-suppress <id>.
    grep -Eq 'NOLINT(NEXTLINE|BEGIN)?([^(A-Za-z]|$)' <<<"$text" && block "bare NOLINT; name the check: NOLINT(<check>)."
    grep -Eq 'cppcheck-suppress\s*$' <<<"$text" && block "cppcheck-suppress needs an id."
    case "$path" in
      */.github/workflows/*|*/.pre-commit-config.yaml|*/.clang-tidy|*/.clang-format|*/platformio.ini)
        grep -Eq 'continue-on-error:\s*true|if:\s*(false|\$\{\{\s*false)|test_ignore|check_skip|--no-verify' <<<"$text" \
          && ask "This weakens CI or a check; confirm the user asked for it."
        ;;
    esac
    ;;
esac
exit 0
