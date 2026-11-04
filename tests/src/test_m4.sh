#!/bin/sh
# GNU m4: macros with arguments, recursion (factorial), ifelse, eval with
# radix output, forloop-style recursion, include, divert/undivert, translit,
# regexp and patsubst, changequote -- the expansion compared with the known one.
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
N=m4
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
M=$L/bin/m4
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
info "$("$M" --version 2>&1 | head -n 1)"
echo 'define(`GREETING'"'"', `Hello from include'"'"')dnl' > inc.m4
cat > t.m4 <<'EOF'
define(`fact', `ifelse(eval($1 <= 1), 1, 1, `eval($1 * fact(decr($1)))')')dnl
define(`upto', `ifelse(eval($1 > $2), 1, , `$1 upto(incr($1), $2)')')dnl
define(`swap', `$2 $1')dnl
include(`inc.m4')dnl
fact(10)
upto(1, 6)
swap(`first', `second')
GREETING
eval(255, 16) eval(5, 2, 8) eval(2**16)
translit(`QNX rocks', `a-z', `A-Z')
regexp(`procnto-smp-instr', `-\([a-z]+\)-', `[\1]')
patsubst(`a.b.c.d', `\.', `/')
len(`neutrino') index(`microkernel', `kern') substr(`realtime', 4, 4)
divert(1)diverted text
divert(0)dnl
before release
undivert(1)dnl
changequote([, ])define([q], [square brackets])q
EOF
cat > exp.txt <<'EOF'
3628800
1 2 3 4 5 6 
second first
Hello from include
ff 00000101 65536
QNX ROCKS
[smp]
a/b/c/d
8 5 time
before release
diverted text
square brackets
EOF
"$M" t.m4 > out.txt 2> err.txt
if cmp -s out.txt exp.txt; then ok "expansion matches (recursion, eval radix, include, divert, regexp, changequote)"
else bad "expansion differs"; diff exp.txt out.txt | head -20; head -5 err.txt; fi
r=$(echo 'define(`x'"'"', `1'"'"')ifdef(`x'"'"', `yes'"'"', `no'"'"') ifdef(`y'"'"', `yes'"'"', `no'"'"')' | "$M")
[ "$r" = "yes no" ] && ok "ifdef from stdin" || bad "ifdef: $r"
r=$(echo 'esyscmd(`echo shell-ok'"'"')' | "$M" | tr -d '\n')
[ "$r" = shell-ok ] && ok "esyscmd runs a shell command" || bad "esyscmd: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
