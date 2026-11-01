#!/bin/sh
# PTPd 2.3.1 (IEEE 1588 PTP daemon): version, the default configuration it
# prints (-O), setting help (-e), configuration checks (-k) of a valid file
# and of one with an out-of-range domain; then, as root, a master-only
# ptpd2 on the loopback interface in unicast mode with clock adjustment
# off (-n), until its status file reports the port in PTP_MASTER state.
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
N=ptpd
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
PD=$L/sbin/ptpd2
r=$(to 20 "$PD" -v 2>&1); echo "$r" | has "ptpd2 version 2.3.1" && ok "ptpd2 -v: version 2.3.1" || bad "-v: $r"
r=$(to 20 "$PD" -O 2>&1)
echo "$r" | has "; PTPDv2 version 2.3.1 default configuration" && echo "$r" | has "ptpengine:domain = 0" \
  && echo "$r" | has "ptpengine:ip_mode = multicast" && ok "-O: default configuration (domain 0, multicast)" || { bad "-O"; echo "$r" | head -8; }
r=$(to 20 "$PD" -e ptpengine:domain 2>&1); echo "$r" | has "PTP domain number." && ok "-e ptpengine:domain: setting help" || bad "-e: $r"
IF=lo0; [ -n "$(ifconfig lo0 2>/dev/null)" ] || IF=$(ifconfig -l 2>/dev/null | tr ' ' '\n' | grep '^lo' | head -n 1)
cat > good.conf <<EOC
ptpengine:interface = $IF
ptpengine:preset = masteronly
ptpengine:ip_mode = unicast
ptpengine:unicast_destinations = 127.0.0.1
ptpengine:domain = 5
clock:no_adjust = Y
global:lock_directory = $T
global:status_file = $T/ptpd.status
global:log_status = Y
global:status_update_interval = 1
global:log_file = $T/ptpd.log
EOC
sed 's/^ptpengine:domain = 5/ptpengine:domain = 500/' good.conf > bad.conf
r=$(to 20 "$PD" -k -c "$T/bad.conf" 2>&1)
echo "$r" | has "Configuration has errors" && echo "$r" | has "ptpengine:domain=500" && echo "$r" | has "0..127" \
  && ok "-k: domain 500 rejected (allowed 0..127)" || { bad "-k bad.conf"; echo "$r" | tail -5; }
r=$(to 20 "$PD" -k -c "$T/good.conf" 2>&1)
if echo "$r" | has "Configuration OK"; then
  ok "-k: the unicast master configuration on $IF is OK"
  if [ "$(id -u)" != 0 ]; then info "not root: ptpd2 needs ports 319/320 -- loopback run skipped"
  else
    "$PD" -C -c "$T/good.conf" > ptpd.out 2>&1 &
    P=$!; track $P
    i=0; while [ $i -lt 40 ]; do grep -q "PTP_MASTER" ptpd.status 2>/dev/null && break; kill -0 $P 2>/dev/null || break; sleep 1; i=$((i+1)); done
    if grep -q "Port state.*PTP_MASTER" ptpd.status 2>/dev/null; then ok "ptpd2 running: port in PTP_MASTER state after ${i} s (status file)"
    else bad "ptpd2 did not reach PTP_MASTER"; cat ptpd.status ptpd.out 2>/dev/null | tail -15; tail -10 ptpd.log 2>/dev/null; fi
    grep -q "Transport.*ipv4, unicast" ptpd.status 2>/dev/null && ok "status: IPv4 unicast transport" || info "status: $(grep Transport ptpd.status 2>/dev/null)"
    kill $P 2>/dev/null; wait $P 2>/dev/null
  fi
else
  bad "-k good.conf"; echo "$r" | tail -8
fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
