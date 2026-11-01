#!/bin/sh
# elvis, the vi clone, without a terminal: its "script" interface (-Gscript)
# runs ex commands from stdin on a file -- substitute, delete, append,
# global, move, write -- and the result is compared with the known text;
# :calc evaluates an expression, :set reports an option (messages end in
# CR LF), and elvtags
# writes a tags file for a C source.
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
N=elvis
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
EV=$L/bin/elvis
[ -d /usr/local/share/elvis ] || { ELVISPATH=$L/share/elvis; export ELVISPATH; }
HOME=$T; export HOME
info "$("$EV" -v 2>&1 | head -n 1)"
printf 'alpha foo\nbeta foo foo\ngamma\ndelta\n' > f.txt
cat > s.ex <<'EOS'
%s/foo/bar/g
3d
$a
epsilon
.
g/^b/s/^/* /
1m$
w
q
EOS
to 30 "$EV" -Gscript f.txt < s.ex > ev.log 2>&1; rc=$?
exp='* beta bar bar
delta
epsilon
alpha bar'
[ $rc -eq 0 ] && [ "$(cat f.txt)" = "$exp" ] && ok "ex script: s///g, d, a, g//, m, w -- the known text" || { bad "ex script (rc $rc)"; cat f.txt; cat ev.log; }
r=$(echo 'calc 6*7' | to 30 "$EV" -Gscript f.txt 2>&1 | tr -d '\015')
[ "$r" = 42 ] && ok ":calc 6*7 = 42" || bad ":calc: $r"
r=$(echo 'set ts?' | to 30 "$EV" -Gscript f.txt 2>&1 | tr -d '\015')
[ "$r" = "tabstop=8" ] && ok ":set ts? -> tabstop=8" || bad ":set: $r"
r=$(printf 'set ts=4\nset ts?\n' | to 30 "$EV" -Gscript f.txt 2>&1 | tr -d '\015')
[ "$r" = "tabstop=4" ] && ok ":set ts=4" || bad ":set ts=4: $r"
printf 'x one\n' > u.txt
printf 's/one/two/\nundo\nw\nq\n' | to 30 "$EV" -Gscript u.txt > /dev/null 2>&1
[ "$(cat u.txt)" = "x one" ] && ok ":undo restores the line" || bad ":undo: $(cat u.txt)"
printf 'int qnx_answer(void)\n{\n  return 42;\n}\nstatic int helper(int x) { return x; }\n' > a.c
to 30 "$L/bin/elvtags" a.c
grep -q "^qnx_answer	a.c	/^int qnx_answer(void)\$/" tags 2>/dev/null && grep -q "^helper	a.c	" tags \
  && ok "elvtags: tags for qnx_answer and helper" || { bad "elvtags"; cat tags 2>/dev/null; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
