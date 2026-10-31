#!/bin/sh
# minicom: runscript (minicom's script language) drives a fake modem on
# stdin/stdout -- send, expect with timeout, goto, print -- and must take the
# OK branch; ascii-xfr sends a text file with CR/LF translation; and minicom
# itself runs in a tmux pane on a pseudo-terminal pair whose master side
# the test holds, and shows text that arrives on the "line".
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
N=minicom
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
fhas() { awk -v p="$2" 'index($0, p) {f=1} END {exit !f}' "$1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
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
HOME=$T; export HOME
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
v=$(to 20 "$L/bin/minicom" --version < /dev/null 2>&1 | head -n 1); case "$v" in *"minicom version"*) ok "--version: $v" ;; *) bad "--version: $v" ;; esac
cat > dial.scr <<'EOF'
print "script started"
send "ATZ"
expect {
  "OK"        goto good
  "ERROR"     goto failed
  timeout 10  goto failed
}
good:
print "modem said OK"
exit 0
failed:
print "no OK from modem"
exit 1
EOF
( sleep 2; printf 'OK\r\n'; sleep 2 ) | to 30 "$L/bin/runscript" dial.scr > sent.out 2> said.out; rc=$?
[ $rc -eq 0 ] && fhas said.out "modem said OK" && ok "runscript: expect matched OK, took the good branch" || { bad "runscript (rc=$rc)"; cat said.out; }
fhas sent.out ATZ && ok "runscript: send wrote ATZ to the line" || bad "runscript send: $(cat sent.out)"
( sleep 2; printf 'BUSY\r\n'; sleep 12 ) | to 30 "$L/bin/runscript" dial.scr > /dev/null 2> said2.out; rc=$?
[ $rc -ne 0 ] && fhas said2.out "no OK from modem" && ok "runscript: timeout branch when the modem never says OK" || { bad "runscript timeout (rc=$rc)"; cat said2.out; }
printf 'line one\nline two\n' > x.txt
to 30 "$L/bin/ascii-xfr" -s -l 0 -c 0 x.txt < /dev/null > x.out 2>/dev/null
h=$(od -An -tx1 x.out | tr -d ' \n')
[ "$h" = 6c696e65206f6e650d0a6c696e652074776f0d0a ] && ok "ascii-xfr -s: LF sent as CR LF" || bad "ascii-xfr: bytes $h"
PM=; PS=
for m in /dev/ptyp? /dev/ptyq? /dev/ptyr? /dev/ptys?; do
  [ -c "$m" ] || continue; s=/dev/tty${m#/dev/pty}; [ -c "$s" ] || continue
  # opening a pty master can block on QNX: probe it under timeout
  to 5 sh -c 'exec 3<>"$1"' sh "$m" </dev/null >/dev/null 2>&1 && { PM=$m; PS=$s; break; }
done
if [ -n "$PM" ] && [ -x "$L/bin/tmux" ]; then
  info "pty pair $PM / $PS"
  TERM=${TERM:-vt100}; [ "$TERM" = dumb ] && TERM=vt100; export TERM
  echo 'set -g default-terminal screen' > tmux.conf
  tm() { to 20 "$L/bin/tmux" -S "$T/sock" -f "$T/tmux.conf" "$@"; }
  qstop_hook() { tm kill-server 2>/dev/null; }
  waitpane() { i=0; while [ $i -lt 20 ]; do tm capture-pane -p -t "$1" 2>/dev/null | awk "$2 {f=1} END {exit !f}" && return 0; sleep 1; i=$((i+1)); done; return 1; }
  ( exec 3<>"$PM" 2>/dev/null; cat <&3 > m.out & c=$!; echo $c >> "$QP"; sleep 6; printf 'HELLO-FROM-THE-LINE\r\n' >&3; sleep 10; kill $c 2>/dev/null ) </dev/null >/dev/null 2>&1 &
  MP=$!; track $MP; sleep 1
  tm new-session -d -s mc -x 80 -y 24 "$L/bin/minicom -o -c off -b 9600 -D $PS" 2> t.err || { bad "tmux session"; cat t.err; }
  waitpane mc '/Welcome to minicom/' && ok "minicom started on $PS (welcome banner)" || { bad "minicom screen"; tm capture-pane -p -t mc 2>&1 | head -6; }
  waitpane mc '/HELLO-FROM-THE-LINE/' && ok "text from the line shown in the minicom window" || { bad "line text not shown"; tm capture-pane -p -t mc 2>&1 | head -8; }
  tm send-keys -t mc 'AT' Enter; sleep 2
  fhas m.out AT && ok "keys typed in minicom reached the line" || bad "typed keys not on the line: $(cat m.out | tr -d '\r')"
  tm kill-server 2>/dev/null; kill $MP 2>/dev/null; wait $MP 2>/dev/null
else info "no free pseudo-terminal or no tmux -- minicom's terminal not exercised"; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
