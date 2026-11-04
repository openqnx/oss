#!/bin/sh
# foot (Wayland terminal): --version; --check-config accepts a valid
# configuration and rejects an unknown key and a bad value; then, against a
# headless labwc compositor, runs a command in a 100x30 terminal: the pty
# size, TERM, and the exit status passed back. (On-screen use is tested
# separately, on the graphics images.)
# Runs in the background with its output to a log, for at most 170 s (a hang
# ends as a FAIL with what it printed so far).
if [ -z "$QNXPORT_INNER" ]; then
  QNXPORT_INNER=/tmp/qnxport-out.$$; export QNXPORT_INNER
  sh "$0" "$@" > "$QNXPORT_INNER" 2>&1 < /dev/null &
  qp=$!; qi=0
  while kill -0 $qp 2>/dev/null && [ $qi -lt 170 ]; do sleep 1; qi=$((qi+1)); done
  if kill -0 $qp 2>/dev/null; then
    for p in $(cat "$QNXPORT_INNER.pids" 2>/dev/null); do kill $p 2>/dev/null; done
    kill $qp 2>/dev/null; sleep 1; kill -KILL $qp 2>/dev/null
    cat "$QNXPORT_INNER"; echo "  FAIL watchdog: still running after 170 s -- stopped"
    echo "FAIL ${0##*/} (watchdog)"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit 1
  fi
  wait $qp; rc=$?; cat "$QNXPORT_INNER"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit $rc
fi
N=foot
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
D=$(cd "$(dirname "$0")" && pwd)   # helpers next to this test
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
# Whatever the test starts stops with it, on every exit path (track PID;
# qstop_hook for anything else); the outer shell above kills them on a hang.
QP=$QNXPORT_INNER.pids; : > "$QP"
track() { echo "$1" >> "$QP"; }
qstop_hook() { :; }
stopall() {
  qstop_hook 2>/dev/null
  for p in $(cat "$QP" 2>/dev/null); do kill "$p" 2>/dev/null; done
  : > "$QP"; [ -n "$T" ] && [ -d "$T" ] && { cd /; rm -rf "$T"; }
}
trap stopall EXIT
trap 'exit 143' TERM
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
F=$L/bin/foot
v=$("$F" --version 2>&1); info "$v"
echo "$v" | grep -q '^foot version: 1\.23\.1' && ok "foot --version: 1.23.1" || bad "--version: $v"
# --check-config: a valid configuration, then one with an unknown key and one with a bad value
mkdir -p cfg
cat > cfg/good.ini <<'EOS'
[main]
font=DejaVu Sans Mono:size=11
pad=4x4
[scrollback]
lines=2000
[colors]
background=101010
foreground=e0e0e0
EOS
printf '[main]\nno-such-option=1\n' > cfg/bad1.ini
printf '[scrollback]\nlines=many\n' > cfg/bad2.ini
"$F" -c cfg/good.ini --check-config > out.txt 2>&1 && ok "--check-config accepts a valid file" || { bad "--check-config good"; cat out.txt; }
"$F" -c cfg/bad1.ini --check-config > out.txt 2>&1; rc=$?
[ $rc -ne 0 ] && grep -q 'no-such-option' out.txt && ok "--check-config rejects an unknown key (names it)" || { bad "bad1: rc $rc"; cat out.txt; }
"$F" -c cfg/bad2.ini --check-config > out.txt 2>&1; rc=$?
[ $rc -ne 0 ] && grep -q 'lines' out.txt && ok "--check-config rejects a non-integer value" || { bad "bad2: rc $rc"; cat out.txt; }
# a real start, headless: labwc's headless backend (pixman renderer) as the
# compositor, foot runs a command in a 100x30 terminal and exits with it
RT=$T/rt; mkdir -m 700 "$RT"
XDG_RUNTIME_DIR=$RT WLR_BACKENDS=headless WLR_RENDERER=pixman "$L/bin/labwc" > labwc.log 2>&1 &
track $!
i=0; while [ ! -S "$RT/wayland-0" ] && [ $i -lt 150 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$RT/wayland-0" ] && ok "labwc up (headless)" || { bad "labwc"; cat labwc.log; }
XDG_RUNTIME_DIR=$RT WAYLAND_DISPLAY=wayland-0 to 60 "$F" -c cfg/good.ini -W 100x30 sh -c "i=0; while [ \$i -lt 10 ] && ! stty -a | grep -q 'rows=30,100'; do sleep 1; i=\$((i+1)); done; stty -a > '$T/stty'; echo \"\$TERM\" > '$T/term'; exit 7" > foot.log 2>&1; rc=$?
# The pty starts at 80x24 and is resized (SIGWINCH) once the compositor has
# configured the window, which can be after the command starts: it waits up
# to 10 s for 100x30. QNX's stty has no "size": "stty -a" reports the window
# as rows=R,C.
sed -n 's/.*rows=\([0-9]*\),\([0-9]*\).*/\1 \2/p' stty 2>/dev/null | head -n 1 > size
info "terminal: $(cat size 2>/dev/null), TERM=$(cat term 2>/dev/null)"
[ "$(cat size 2>/dev/null)" = "30 100" ] && ok "foot ran the command in a 100x30 terminal (pty size)" || { bad "pty size '$(cat size 2>/dev/null)'"; tail -n 20 foot.log; }
case "$(cat term 2>/dev/null)" in foot|xterm-256color) ok "TERM=$(cat term) inside the terminal" ;; *) bad "TERM '$(cat term 2>/dev/null)'" ;; esac
[ $rc -eq 7 ] && ok "foot exits with the command's status (7)" || bad "exit status $rc"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
