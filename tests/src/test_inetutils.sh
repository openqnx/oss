#!/bin/sh
# GNU inetutils (the subset ported): telnetd, started for one connection
# by test_inetutils_inetd as inetd would (the socket on its stdin), runs a
# shell on a pty for the telnet client over loopback: a command typed into
# telnet comes back executed. ping (raw ICMP, root) and traceroute (UDP
# probes) to 127.0.0.1, hostname against uname -n, and the clients'
# versions. ftpd needs a real user login and is not exercised.
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
N=inetutils
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
vf=0
for p in bin/telnet bin/ftp bin/ping bin/traceroute bin/hostname bin/whois bin/logger libexec/telnetd libexec/ftpd; do
  r=$(to 20 "$L/$p" --version 2>&1 | head -n 1)
  case "$r" in *"(GNU inetutils) 2.6"*) : ;; *) bad "$p --version: $r"; vf=1 ;; esac
done
[ $vf -eq 0 ] && ok "telnet, ftp, ping, traceroute, hostname, whois, logger, telnetd, ftpd: GNU inetutils 2.6"
r=$(to 20 "$L/bin/hostname" 2>&1); [ "$r" = "$(uname -n)" ] && ok "hostname = uname -n ($r)" || bad "hostname: $r vs $(uname -n)"
P=$((23000 + $$ % 2000))
"$D/test_inetutils_inetd" $P "$L/libexec/telnetd" -h -E /bin/sh > inetd.out 2>&1 &
I=$!; track $I
i=0; while [ $i -lt 10 ]; do grep -q listening inetd.out 2>/dev/null && break; sleep 1; i=$((i+1)); done
# telnetd looks up the local host's full name (getaddrinfo of the
# hostname) before it says anything; a target whose /etc/hosts does not
# name it asks its DNS server, which on the test VMs takes 30 s to fail.
# Input that arrives before the shell runs can be lost, so the command is
# typed again every 5 s until its output comes back (at most 120 s).
: > tn.out
t0=$(date +%s)
( i=0; while [ $i -lt 24 ] && ! grep -q 'TELNET-42' tn.out 2>/dev/null; do echo 'echo TEL""NET-42'; sleep 5; i=$((i+1)); done
  echo exit; sleep 3 ) | to 140 "$L/bin/telnet" 127.0.0.1 $P > tn.out 2>&1
r=$(tr -d '\015' < tn.out); info "telnet session: $(( $(date +%s) - t0 )) s"
echo "$r" | has "Connected to 127.0.0.1." && ok "telnet connected to telnetd on 127.0.0.1:$P" || { bad "telnet connect"; echo "$r"; cat inetd.out; }
echo "$r" | grep -q 'TELNET-42' && ok "a command typed into telnet ran in telnetd's pty shell (echo TEL\"\"NET-42 printed TELNET-42)" || { bad "telnet session"; echo "$r"; }
kill $I 2>/dev/null; wait $I 2>/dev/null
if [ "$(id -u)" = 0 ]; then
  r=$(to 30 "$L/bin/ping" -c 2 127.0.0.1 2>&1)
  echo "$r" | has "2 packets transmitted, 2 packets received" && ok "ping -c 2 127.0.0.1: 2 transmitted, 2 received" || { bad "ping"; echo "$r"; }
  # (inetutils' traceroute prints addresses unless --resolve-hostnames: no -n)
  r=$(to 60 "$L/bin/traceroute" -m 3 127.0.0.1 2>&1)
  echo "$r" | awk '$1 == 1 && $2 == "127.0.0.1" {f=1} END {exit !f}' && ok "traceroute 127.0.0.1: hop 1 is 127.0.0.1" || { bad "traceroute"; echo "$r"; }
else info "not root: ping and traceroute need raw sockets -- skipped"; fi
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
