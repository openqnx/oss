#!/bin/sh
# picocom on a pseudo-terminal pair (QNX devc-pty), standing in for a serial
# line: the test holds the master side. picocom opens the slave as its
# port, configures it (115200 8N1, printed back), sends an init string that
# arrives at the master; then in a two-way session text written by the
# master reaches picocom's output and picocom's input reaches the master.
# Run the test itself in the background, its output to a log, and wait at
# most 170 s: nothing it leaves behind can hold the runner's output, and a
# hang ends as a FAIL with whatever the test printed so far.
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
N=picocom
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
PC=$L/bin/picocom
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
v=$("$PC" --help 2>&1 | head -n 1); case "$v" in picocom*) ok "--help: $v" ;; *) bad "--help: $v" ;; esac
PM=; PS=
for m in /dev/ptyp? /dev/ptyq? /dev/ptyr? /dev/ptys?; do
  [ -c "$m" ] || continue; s=/dev/tty${m#/dev/pty}; [ -c "$s" ] || continue
  # opening a pty master can block on QNX: probe it under timeout
  to 5 sh -c 'exec 3<>"$1"' sh "$m" </dev/null >/dev/null 2>&1 && { PM=$m; PS=$s; break; }
done
master() { # file to record into, text to send after 2 s, seconds to stay
  ( exec 3<>"$PM" 2>/dev/null; cat <&3 > "$1" & c=$!; echo $c >> "$QP"; sleep 2; [ -n "$2" ] && printf '%s\r\n' "$2" >&3; sleep $3; kill $c 2>/dev/null ) </dev/null >/dev/null 2>&1 &
}
if [ -z "$PM" ]; then
  r=$("$PC" -X /dev/qnx-no-such-port 2>&1)
  case "$r" in *"cannot open"*|*"FATAL"*) ok "missing port reported: $(echo "$r" | tail -n 1)" ;; *) bad "missing port: $r" ;; esac
  info "no free pseudo-terminal (/dev/ptyp*) -- port I/O not exercised"
else
  info "pty pair $PM / $PS"
  master m1.out "" 4; MP=$!; track $MP; sleep 1
  r=$(to 30 "$PC" -b 115200 -d 8 -p n -f n -X -t 'QNX-PICOCOM-INIT' "$PS" 2>&1)
  wait $MP 2>/dev/null
  case "$r" in *"baudrate is"*": 115200"*) ok "port configured: baudrate 115200" ;; *) bad "settings: $(echo "$r" | head -n 5)" ;; esac
  case "$r" in *"databits are"*": 8"*) ok "8 data bits, no parity" ;; *) bad "databits" ;; esac
  fhas m1.out QNX-PICOCOM-INIT && ok "init string arrived at the master side" || bad "init string not received: $(cat m1.out)"
  master m2.out FROM-MASTER 6; MP=$!; track $MP; sleep 1
  ( sleep 3; printf 'TO-PORT\r' ) | to 30 "$PC" -q -b 9600 --exit-after 2500 "$PS" > p2.out 2>&1
  wait $MP 2>/dev/null
  fhas p2.out FROM-MASTER && ok "master -> port -> picocom output: FROM-MASTER" || bad "picocom output: $(cat p2.out | tr -d '\r' | head -n 3)"
  fhas m2.out TO-PORT && ok "picocom input -> port -> master: TO-PORT" || bad "master received: $(cat m2.out | tr -d '\r')"
fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
