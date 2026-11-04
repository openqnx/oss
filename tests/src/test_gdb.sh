#!/bin/sh
# GDB, natively on the target, in batch mode, on an installed program (GNU
# m4): reads its symbol table (functions, addresses, disassembly), then runs
# it under the debugger -- stops at a breakpoint in define_macro, prints a
# backtrace through main and the pc, continues to a normal exit, and the
# program's own output is right.
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
N=gdb
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
GD=$L/bin/gdb; M4=$L/bin/m4
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
HOME=$T; TERM=dumb; export HOME TERM
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
gb() { to 120 "$GD" -nx -batch -iex 'set pagination off' -iex 'set confirm off' -iex 'set width 0' "$@" < /dev/null; }
info "$("$GD" --version 2>&1 | head -n 1)"
[ -x "$M4" ] || { bad "debuggee $M4 not installed"; echo "FAIL $N ($fail check(s) failed)"; exit 1; }
# The installed m4 is stripped of DWARF (packages carry no debug info), so
# the static checks use the ELF symbol table: functions, addresses and the
# disassembler.
r=$(gb -ex 'info functions ^define_macro$' -ex 'info address main' -ex 'x/3i main' "$M4" 2>&1)
echo "$r" | has "define_macro" && ok "info functions finds define_macro (symbol table)" || { bad "info functions"; echo "$r" | head -5; }
echo "$r" | awk '/Symbol "main" is at 0x[0-9a-f]+/ {f=1} END {exit !f}' && ok "info address main" || bad "info address main"
echo "$r" | awk '/<main\+?[0-9]*>:/ {f=1} END {exit !f}' && ok "x/3i main disassembles" || { bad "x/3i main"; echo "$r" | tail -4; }
gb -ex 'info line main' "$M4" 2>&1 | has "No line number information" && info "m4 has no line-number information (stripped package)"
printf 'define(`qnxmacro'"'"', `forty-two'"'"')qnxmacro\n' > in.m4
r=$(gb -ex 'break define_macro' -ex 'run' -ex 'bt' -ex 'info registers pc' -ex 'continue' --args "$M4" in.m4 2>&1)
echo "$r" | awk '/^Breakpoint 1 at/ {f=1} END {exit !f}' && ok "breakpoint set on define_macro" || { bad "break"; echo "$r" | head -5; }
echo "$r" | awk '/^Breakpoint 1, .*define_macro/ {f=1} END {exit !f}' && ok "run: stopped at the breakpoint" || { bad "did not stop at the breakpoint"; echo "$r" | tail -8; }
echo "$r" | awk '/^#0 .*define_macro/ {a=1} /^#[0-9]+ .*main \(/ {b=1} END {exit !(a && b)}' && ok "bt: define_macro ... main" || bad "backtrace"
echo "$r" | has "forty-two" && ok "program output after continue: forty-two" || bad "program output"
echo "$r" | awk '/\[Inferior 1 \((process|pid) [0-9]+\) exited normally\]/ {f=1} END {exit !f}' && ok "inferior exited normally" || { bad "inferior exit"; echo "$r" | tail -4; }
r=$(gb -ex 'print sizeof(long)' -ex 'print 6*7' -ex 'print/x 255' 2>&1)
echo "$r" | has '$2 = 42' && echo "$r" | has '$3 = 0xff' && ok "expression evaluator (print 6*7, print/x 255)" || { bad "print"; echo "$r" | head -4; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
