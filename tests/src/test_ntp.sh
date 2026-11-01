#!/bin/sh
# The reference ntpd (ntp.org) on loopback with its own local clock driver
# (127.127.1.0, stratum 10) as the only reference: the system clock against
# itself, so the corrections it slews (-x: never steps; no kernel PLL, which
# QNX does not have) through ClockAdjust() are zero. ("disable ntp" would keep
# the clock untouched but also keeps ntpd unsynchronised: stratum 16, refid
# INIT, which no client accepts.) ntpd holds the local clock back for
# "orphanwait" (300 s by default; a value below 1 means the default): 1 s
# here. ntpq reads the system and
# peer variables, ntpdc (mode 7) the system info, and ntpdate -q queries
# the server once it is synchronised (stratum 11). ntp-keygen -M writes a
# symmetric key file. ntpd needs UDP port 123, so the daemon part needs
# root (as the test runner is on the target); ntpsec and chrony are
# separate and not involved.
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
N=ntp
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
NB=$L/opt/ntp/bin
info "$("$NB/ntpq" --version 2>&1 | head -n 1)"
r=$(to 30 "$NB/ntpd" --version 2>&1); echo "$r" | has "ntpd 4.2.8p18" && ok "ntpd --version: 4.2.8p18" || bad "ntpd --version: $r"
r=$(to 30 "$NB/ntp-keygen" -M 2>&1); rc=$?
[ $rc -eq 0 ] && [ -f ntp.keys ] && grep -q '^ 1 MD5 ' ntp.keys && ok "ntp-keygen -M: ntp.keys with MD5 key 1" || { bad "ntp-keygen -M (rc $rc): $r"; }
if [ "$(id -u)" != 0 ]; then info "not root: the ntpd daemon part needs port 123 -- skipped"
else
cat > ntp.conf <<EOC
server 127.127.1.0 minpoll 4 maxpoll 4
fudge 127.127.1.0 stratum 10
disable kernel
tos orphanwait 1
enable mode7
interface ignore wildcard
interface listen 127.0.0.1
restrict default ignore
restrict 127.0.0.1
driftfile $T/ntp.drift
pidfile $T/ntpd.pid
logfile $T/ntpd.log
EOC
"$NB/ntpd" -n -x -c "$T/ntp.conf" > ntpd.out 2>&1 &
P=$!; track $P
q() { to 20 "$NB/ntpq" -n -c "$1" 127.0.0.1 2>&1; }
i=0; while [ $i -lt 20 ]; do r=$(q "rv 0 version"); case "$r" in *"ntpd 4.2.8p18"*) break ;; esac; kill -0 $P 2>/dev/null || break; sleep 1; i=$((i+1)); done
case "$r" in *'version="ntpd 4.2.8p18'*) ok "ntpq rv: the server answers mode 6 (version=ntpd 4.2.8p18)" ;; *) bad "ntpq rv: $r"; cat ntpd.out ntpd.log 2>/dev/null | tail -15 ;; esac
r=$(to 20 "$NB/ntpq" -np 127.0.0.1 2>&1)
echo "$r" | has "remote           refid      st t when poll reach" && echo "$r" | awk '$1 ~ /127\.127\.1\.0$/ && $2 == ".LOCL." && $3 == 10 && $4 == "l" {f=1} END {exit !f}' \
  && ok "ntpq -p: the local clock 127.127.1.0 .LOCL. stratum 10" || { bad "ntpq -p"; echo "$r"; }
r=$(to 20 "$NB/ntpdc" -n -c sysinfo 127.0.0.1 2>&1)
echo "$r" | has "system peer:" && echo "$r" | has "stratum:" && ok "ntpdc sysinfo (mode 7)" || { bad "ntpdc sysinfo"; echo "$r" | head -5; }
# Synchronised to the local clock after a few 16 s polls.
i=0; while [ $i -lt 110 ]; do r=$(q "rv 0 stratum"); case "$r" in *"stratum=11"*) break ;; esac; kill -0 $P 2>/dev/null || break; sleep 2; i=$((i+2)); done
case "$r" in *"stratum=11"*) ok "synchronised to the local clock after ${i} s: stratum 11" ;; *) bad "not synchronised: $r"; q "pe" ;; esac
r=$(q "rv 0 refid"); case "$r" in *"refid=LOCAL(0)"*|*"refid=127.127.1.0"*) ok "system refid: the local clock" ;; *) bad "refid: $r" ;; esac
r=$(to 30 "$NB/ntpdate" -qu 127.0.0.1 2>&1)
echo "$r" | has "server 127.0.0.1, stratum 11, offset" && ok "ntpdate -q: $(echo "$r" | awk '/^server/ {print; exit}')" || { bad "ntpdate -q"; echo "$r"; }
kill $P 2>/dev/null; wait $P 2>/dev/null
fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
