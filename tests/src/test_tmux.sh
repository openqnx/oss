#!/bin/sh
# tmux: a detached server on a private socket -- a session runs a shell, gets
# keystrokes with send-keys and its pane is captured; windows and panes are
# created and counted, a format is expanded, paste buffers are set and read
# back, and the server is killed.
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
N=tmux
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
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
TERM=${TERM:-vt100}; [ "$TERM" = dumb ] && TERM=vt100; export TERM
echo 'set -g default-terminal screen' > tmux.conf
tm() { to 20 "$L/bin/tmux" -S "$T/sock" -f "$T/tmux.conf" "$@"; }
qstop_hook() { tm kill-server 2>/dev/null; }
waitpane() { # target, awk pattern: wait up to 15 s for the pane to show it
  i=0; while [ $i -lt 15 ]; do tm capture-pane -p -t "$1" 2>/dev/null | awk "$2 {f=1} END {exit !f}" && return 0; sleep 1; i=$((i+1)); done; return 1; }
info "$("$L/bin/tmux" -V 2>&1)"
# -vv: the server and client write tmux-server-PID.log / tmux-client-PID.log
# into the current directory, shown if the server does not come up.
tm -vv new-session -d -s qnx -x 100 -y 30 /bin/sh 2> err.txt && ok "new-session -d (100x30, /bin/sh)" || {
  bad "new-session"; head -3 err.txt
  for lg in tmux-server-*.log tmux-client-*.log; do [ -f "$lg" ] && { echo "  --- $lg (last lines)"; tail -n 15 "$lg"; }; done
  ( EVENT_NOEPOLL=1; export EVENT_NOEPOLL; tm new-session -d -s probe /bin/sh 2>/dev/null ) && info "with EVENT_NOEPOLL=1 the server starts: libevent's epoll backend is the problem"
  tm kill-server 2>/dev/null; }
r=$(tm display-message -p -t qnx '#{session_name} #{window_width}x#{window_height} #{pane_current_command}' 2>&1)
case "$r" in "qnx 100x30"*) ok "display-message format: $r" ;; *) bad "display-message: $r" ;; esac
tm send-keys -t qnx 'echo "sum=$((6*7)) os=$(uname -s)"' Enter
waitpane qnx '/^sum=42 os=/' && ok "send-keys ran a command; capture-pane shows sum=42" || { bad "capture-pane"; tm capture-pane -p -t qnx 2>&1 | head -5; }
tm new-window -t qnx -n second "printf 'second-window\n'; sleep 60"
tm split-window -t qnx:0 "printf 'split-pane\n'; sleep 60"
[ "$(tm list-windows -t qnx 2>/dev/null | wc -l | tr -d ' ')" = 2 ] && ok "list-windows: 2" || bad "list-windows: $(tm list-windows -t qnx 2>&1)"
[ "$(tm list-panes -t qnx:0 2>/dev/null | wc -l | tr -d ' ')" = 2 ] && ok "split-window: 2 panes in window 0" || bad "list-panes"
waitpane qnx:second '/second-window/' && ok "second window's command output captured" || bad "second window output"
tm set-buffer -b qb "buffer from QNX" && [ "$(tm show-buffer -b qb 2>&1)" = "buffer from QNX" ] && ok "set-buffer / show-buffer" || bad "buffers"
tm rename-session -t qnx qnx2 && tm has-session -t qnx2 2>/dev/null && ok "rename-session / has-session" || bad "rename-session"
tm kill-server 2>/dev/null; sleep 1
tm has-session -t qnx2 2>/dev/null && bad "server still running after kill-server" || ok "kill-server"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
