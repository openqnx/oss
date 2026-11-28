#!/bin/sh
# oss-greeter (the log-in screen of the desktop): what is installed and how it
# is set up -- the greeter and its session script, the PAM policy naming QNX's
# module by its absolute path for the four facilities (as swaylock's), the
# labwc configuration with no menu and a binding in each section (labwc
# loads its defaults -- a terminal, close, next window -- only when there is
# none); qnx-desktop-start knows DESKTOP_GREETER. Then the greeter itself on
# a headless labwc of its own (oss-greeter-session --headless): it comes up
# on the layer shell, lists root, keeps running, and writes no answer while
# nobody logs in. Typing a password needs QNX Screen: the desktop harness
# does that part (wrong password, log-in, log-out back to the greeter).
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL oss_greeter (qnxport-timeout.sh missing)"; exit 1; }
echo "== oss-greeter =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
G=$P/libexec/oss-greeter/oss-greeter; GS=$P/libexec/oss-greeter/oss-greeter-session
POL=$P/etc/pam.d/oss-greeter; RC=$P/share/oss-greeter/labwc/rc.xml
T=$(mktemp -d /tmp/qnxport-greeter.XXXXXX) || { echo "FAIL oss-greeter (mktemp)"; exit 1; }
GP=""
trap '[ -n "$GP" ] && kill $GP 2>/dev/null; cd /; rm -rf "$T"' EXIT  # GP is the test's own labwc (the script execs it)
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

[ -x "$G" ] && [ -x "$GS" ] && ok "oss-greeter and oss-greeter-session installed" || bad "not installed ($G, $GS)"
if ldd "$G" > ldd.out 2>&1; then
  grep -q 'libpam\.so' ldd.out && grep -q 'libgtk-layer-shell' ldd.out && ok "oss-greeter loads (ldd), with OpenPAM and gtk-layer-shell" \
    || bad "oss-greeter: libpam or libgtk-layer-shell missing from ldd"
else bad "oss-greeter: $(grep -i 'not found\|cannot' ldd.out | head -n 2)"; fi
if [ -f "$POL" ]; then
  n=0; for f in auth account session password; do
    grep -q "^$f[[:space:]].*[[:space:]]/system/lib/dll/pam/pam_qnx\.so" "$POL" && n=$((n+1)); done
  [ $n -eq 4 ] && ok "PAM policy: pam_qnx.so by absolute path for auth, account, session, password" || bad "PAM policy has $n of 4 facilities on /system/lib/dll/pam/pam_qnx.so"
  [ -f /system/lib/dll/pam/pam_qnx.so ] && echo "  info QNX's pam_qnx.so is there" || echo "  info no /system/lib/dll/pam/pam_qnx.so on this image: log-in would fail (Authentication is not available)"
else bad "no PAM policy $POL"; fi
if [ -f "$RC" ]; then
  grep -q '<keybind ' "$RC" && grep -q '<mousebind ' "$RC" && ! grep -q -i -E 'Execute|ShowMenu|<default' "$RC" \
    && ok "greeter's labwc: one inert key and mouse binding, no Execute/ShowMenu/default bindings" \
    || bad "greeter's labwc rc.xml lets something through: $(grep -i -E 'Execute|ShowMenu|default' "$RC" | head -n 2)"
else bad "no $RC"; fi
Q=$(command -v qnx-desktop-start 2>/dev/null || echo $P/bin/qnx-desktop-start)
if [ -f "$Q" ]; then grep -q 'DESKTOP_GREETER' "$Q" && ok "qnx-desktop-start knows DESKTOP_GREETER" || bad "qnx-desktop-start has no DESKTOP_GREETER"
else echo "  info qnx-desktop-start not installed (qnx-desktop-core)"; fi

# ---- the greeter on a headless labwc of its own ----
export XDG_RUNTIME_DIR=$T/rt; mkdir -m 700 "$XDG_RUNTIME_DIR"
"$GS" --headless --result "$T/result" --last-user "$T/last-user" > gs.out 2>&1 &
GP=$!
i=0; while [ $i -lt 30 ] && ! pidin -F '%N' 2>/dev/null | grep -q '^oss-greeter'; do sleep 1; i=$((i+1)); done
sleep 4
if kill -0 $GP 2>/dev/null && pidin -F '%N' | grep -q '^oss-greeter'; then
  ok "the greeter runs on its labwc (headless) after ${i}+4 s"
else bad "the greeter did not come up: $(tail -n 5 $XDG_RUNTIME_DIR/labwc.log 2>/dev/null)"; fi
grep -q 'no layer-shell' $XDG_RUNTIME_DIR/labwc.log 2>/dev/null && bad "no layer-shell: a plain window" || ok "on the layer shell (overlay, full screen)"
[ -e "$T/result" ] && bad "an answer without a log-in: $(cat "$T/result")" || ok "no answer written while nobody logs in"
grep -q '^root:' /etc/passwd && echo "  info users it lists: $(awk -F: '($3 == 0 || ($3 >= 1000 && $3 < 60000 && $1 != "nobody")) && $7 !~ /(false|nologin)/ {print $1}' /etc/passwd | tr '\n' ' ')"
kill $GP 2>/dev/null; sleep 1; GP=""

[ $fail -eq 0 ] && { echo "PASS oss-greeter"; exit 0; }
echo "FAIL oss-greeter ($fail check(s) failed)"; exit 1
