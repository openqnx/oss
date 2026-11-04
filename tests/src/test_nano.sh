#!/bin/sh
# GNU nano: edit a file for real -- nano runs in a tmux pane (nano needs a
# terminal), keystrokes insert text at the start and the end of a line,
# Ctrl-O writes the file, Ctrl-X quits; the file on disk must hold the edit.
# Also: --version, and a second session that searches with Ctrl-W.
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
N=nano
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
NA=$L/bin/nano
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
v=$("$NA" --version 2>&1 | head -n 1); case "$v" in *"GNU nano"*) ok "--version: $(echo $v)" ;; *) bad "--version: $v" ;; esac
TERM=${TERM:-vt100}; [ "$TERM" = dumb ] && TERM=vt100; export TERM; HOME=$T; export HOME
echo 'set -g default-terminal screen' > tmux.conf
tm() { to 20 "$L/bin/tmux" -S "$T/sock" -f "$T/tmux.conf" "$@"; }
qstop_hook() { tm kill-server 2>/dev/null; }
waitpane() { i=0; while [ $i -lt 20 ]; do tm capture-pane -p -t "$1" 2>/dev/null | awk "$2 {f=1} END {exit !f}" && return 0; sleep 1; i=$((i+1)); done; return 1; }
waitgone() { i=0; while [ $i -lt 20 ]; do tm has-session -t "$1" 2>/dev/null || return 0; sleep 1; i=$((i+1)); done; return 1; }
printf 'first line\nsecond line\nthird line\n' > f.txt
tm new-session -d -s ed -x 80 -y 24 "$NA -I f.txt" 2> err.txt || { bad "tmux session for nano"; head -3 err.txt; }
waitpane ed '/first line/' && ok "nano shows the file in its window" || { bad "nano screen"; tm capture-pane -p -t ed 2>&1 | head -5; }
tm send-keys -t ed 'TOP '
tm send-keys -t ed Down C-e ' END'
sleep 1
tm send-keys -t ed C-o; waitpane ed '/File Name to Write/' || info "write prompt not seen"
tm send-keys -t ed Enter; sleep 1
tm send-keys -t ed C-x
waitgone ed && ok "Ctrl-O, Ctrl-X: nano saved and exited" || { bad "nano did not exit"; tm capture-pane -p -t ed 2>&1 | tail -4; }
exp='TOP first line
second line END
third line'
[ "$(cat f.txt)" = "$exp" ] && ok "file on disk holds both edits" || { bad "edited file"; cat f.txt; }
i=0; while [ $i -lt 120 ]; do echo "filler line $i"; i=$((i+1)); done > s.txt; echo "the needle line" >> s.txt
tm new-session -d -s se -x 80 -y 24 "$NA -I -l s.txt"
waitpane se '/filler line 0/' || info "search session did not start"
tm send-keys -t se C-w 'needle' Enter
waitpane se '/the needle line/' && ok "Ctrl-W search scrolled to line 121" || { bad "search"; tm capture-pane -p -t se 2>&1 | tail -4; }
tm send-keys -t se C-x
tm kill-server 2>/dev/null
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
