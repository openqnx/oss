#!/bin/sh
# chrony on loopback, without touching the clock (-x): chronyd parses and
# prints a config (-p); a chronyd NTP server with a local reference answers
# chronyc's monitoring queries (tracking, serverstats) over UDP; and a
# second chronyd in client mode (-Q) measures the offset against it once.
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
N=chrony
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
CD=$L/sbin/chronyd; CC=$L/bin/chronyc
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
info "$("$CD" -v 2>&1 | head -n 1)"
NP=$((12300 + $$ % 2000)); CP=$((NP + 2000))
cat > server.conf <<EOF
port $NP
cmdport $CP
bindaddress 127.0.0.1
bindcmdaddress 127.0.0.1
bindcmdaddress $T/cmd/chronyd.sock
allow 127.0.0.1
local stratum 8
pidfile $T/server.pid
driftfile $T/drift
EOF
mkdir cmd; chmod 700 cmd
r=$("$CD" -p -f server.conf 2>&1)
echo "$r" | has "local stratum 8" && echo "$r" | has "port $NP" && ok "chronyd -p: config parsed and printed" || { bad "chronyd -p"; echo "$r" | head -5; }
"$CD" -x -d -f "$T/server.conf" > server.log 2>&1 &
S=$!; track $S
cc() { to 20 "$CC" -h 127.0.0.1 -p $CP -n "$@" 2>&1; }
i=0; while [ $i -lt 20 ]; do r=$(cc tracking); case "$r" in *"Reference ID"*7F7F0101*) break ;; esac; sleep 1; i=$((i+1)); done
case "$r" in *"Reference ID"*7F7F0101*) ok "chronyc tracking: local reference 7F7F0101" ;; *) bad "chronyc tracking: $r"; tail -5 server.log ;; esac
echo "$r" | awk '/^Stratum/ && $3 == 8 {f=1} END {exit !f}' && ok "tracking: stratum 8" || bad "tracking stratum"
# serverstats is privileged: chronyc must use the Unix command socket for it.
ccs() { to 20 "$CC" -h "$T/cmd/chronyd.sock" -n "$@" 2>&1; }
r=$(cc sources); case "$r" in *"MS Name/IP"*|*"Number of sources = 0"*|*"210 Number of sources"*) ok "chronyc sources over UDP (no upstream sources)" ;; *) bad "chronyc sources: $r" ;; esac
r=$(ccs serverstats); echo "$r" | has "NTP packets received" && ok "chronyc serverstats over the Unix command socket" || bad "serverstats: $r"
cat > client.conf <<EOF
server 127.0.0.1 port $NP iburst minpoll -2 maxpoll -2
port 0
cmdport 0
pidfile $T/client.pid
EOF
r=$(to 60 "$CD" -x -Q -t 30 -f "$T/client.conf" 2>&1)
case "$r" in *"System clock wrong by"*|*"System clock"*"offset"*) ok "chronyd -Q measured the offset over NTP: $(echo "$r" | awk '/System clock/ {print; exit}')" ;; *) bad "chronyd -Q: $r" ;; esac
r=$(ccs serverstats); n=$(echo "$r" | awk -F: '/NTP packets received/ {gsub(/ /, "", $2); print $2}')
[ "${n:-0}" -gt 0 ] && ok "server counted $n NTP packets from the client" || bad "server NTP packet count: $n"
kill $S 2>/dev/null; wait $S 2>/dev/null
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
