#!/bin/sh
# less: as a filter (not a terminal) it copies its input; lesskey compiles a
# key binding file (binary magic checked); lessecho quotes arguments; and in
# a tmux pane (a real terminal) less pages a file, searches with /pattern
# and shows the match at the top with line numbers, then quits on q.
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
N=less
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
LS=$L/bin/less
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
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
LESSHISTFILE=-; LESS=; export LESSHISTFILE LESS
info "$("$LS" --version 2>&1 | head -n 1)"
i=1; while [ $i -le 200 ]; do if [ $i = 150 ]; then echo "needle at one-fifty"; else echo "line $i"; fi; i=$((i+1)); done > f.txt
"$LS" f.txt | cmp -s - f.txt && ok "filter mode: output identical to input" || bad "filter mode"
printf '#command\n\\r forw-line\n^K back-line\n' > keys.txt
"$L/bin/lesskey" -o keys.bin keys.txt > lk.log 2>&1 && [ "$(od -An -tx1 -N4 keys.bin | tr -d ' \n')" = 004d2b47 ] && ok "lesskey: compiled file with magic 00 4d 2b 47" || { bad "lesskey"; cat lk.log; }
r=$("$L/bin/lessecho" -a '-o<' '-c>' x 'a b' 2>&1)
[ "$r" = "<x> <a b>" ] && ok "lessecho quoting" || bad "lessecho: $r"
TERM=${TERM:-vt100}; [ "$TERM" = dumb ] && TERM=vt100; export TERM
echo 'set -g default-terminal screen' > tmux.conf
tm() { to 20 "$L/bin/tmux" -S "$T/sock" -f "$T/tmux.conf" "$@"; }
qstop_hook() { tm kill-server 2>/dev/null; }
waitpane() { i=0; while [ $i -lt 20 ]; do tm capture-pane -p -t "$1" 2>/dev/null | awk "$2 {f=1} END {exit !f}" && return 0; sleep 1; i=$((i+1)); done; return 1; }
if [ -x "$L/bin/tmux" ]; then
  tm new-session -d -s pg -x 80 -y 24 "$LS -N f.txt" 2>/dev/null
  waitpane pg 'NR==1 && /1 line 1$/' && ok "pager: first screen with line numbers" || { bad "pager first screen"; tm capture-pane -p -t pg 2>&1 | head -3; }
  tm send-keys -t pg '/needle' Enter
  waitpane pg 'NR==1 && /150 needle at one-fifty/' && ok "/needle: match at the top of the screen" || { bad "search"; tm capture-pane -p -t pg 2>&1 | head -3; }
  tm send-keys -t pg G
  waitpane pg '/200 line 200/' && ok "G: end of file" || bad "G"
  tm send-keys -t pg q; sleep 2
  tm has-session -t pg 2>/dev/null && bad "q did not quit" || ok "q quits"
  tm kill-server 2>/dev/null
else info "tmux not installed -- pager mode not exercised"; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
