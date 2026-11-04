#!/bin/sh
# bc and dc (GNU bc): arbitrary precision -- pi to 20 places with the math
# library, 2^100, a square root, a user-defined recursive function, base
# conversion; dc computes 2^64 and a stack sum. Known answers compared.
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
N=bc
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
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
BC=$L/bin/bc; DC=$L/bin/dc
info "$("$BC" --version 2>&1 | head -n 1)"
t() { # description, program, expected
  r=$(printf '%s\n' "$2" | BC_LINE_LENGTH=0 "$BC" -lq 2>&1)
  [ "$r" = "$3" ] && ok "bc: $1 = $(echo $3)" || bad "bc: $1: got '$r', want '$3'"
}
t "pi (scale=20, 4*a(1))" 'scale=20; 4*a(1)' 3.14159265358979323844
t "2^100" '2^100' 1267650600228229401496703205376
t "sqrt(2) (scale=10)" 'scale=10; sqrt(2)' 1.4142135623
t "e^1 (scale=15)" 'scale=15; e(1)' 2.718281828459045
t "recursive factorial 25" 'define f(n) { if (n <= 1) return (1); return (n * f(n-1)); }
f(25)' 15511210043330985984000000
t "obase=16 of 3735928559" 'obase=16; 3735928559' DEADBEEF
t "ibase=2 of 101010" 'ibase=2; 101010' 42
t "integer division and modulo" 'scale=0; 1000003 / 7; 1000003 % 7' "142857
4"
r=$(echo '2 64 ^ p' | "$DC" 2>&1)
[ "$r" = 18446744073709551616 ] && ok "dc: 2 64 ^ = $r" || bad "dc: 2^64: $r"
r=$(echo '1 2 3 4 5 + + + + p' | "$DC" 2>&1)
[ "$r" = 15 ] && ok "dc: stack sum 1..5 = 15" || bad "dc: stack sum: $r"
r=$(echo '10 k 1 3 / p' | "$DC" 2>&1)
[ "$r" = .3333333333 ] && ok "dc: precision 10, 1/3 = $r" || bad "dc: 1/3: $r"
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
