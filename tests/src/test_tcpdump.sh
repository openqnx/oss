#!/bin/sh
# tcpdump on the libpcap port, offline: test_tcpdump_mkpcap writes a
# capture with a DNS query, an ICMP echo request and a TCP SYN (valid
# checksums); tcpdump decodes each to its known line, verifies the
# checksums (-vv), applies BPF filters, writes a filtered capture (-w) and
# reads it back, hex-dumps a packet (-X), and compiles a filter (-d).
# Live capture needs a network interface and root, so it is not tried.
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
N=tcpdump
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
TD=$L/bin/tcpdump
info "$("$TD" --version 2>&1 | head -n 2 | tr '\n' ' ')"
"$D/test_tcpdump_mkpcap" t.pcap && [ -s t.pcap ] && ok "capture file written ($(wc -c < t.pcap | tr -d ' ') bytes)" || bad "test_tcpdump_mkpcap"
r=$(to 30 "$TD" -ttnr t.pcap 2>/dev/null)
exp="1700000000.000000 IP 10.0.0.1.40000 > 10.0.0.2.53: 4660+ A? qnx.example. (30)
1700000001.000000 IP 10.0.0.1 > 10.0.0.2: ICMP echo request, id 1, seq 1, length 16
1700000002.000000 IP 10.0.0.1.40001 > 10.0.0.2.80: Flags [S], seq 1000, win 65535, length 0"
[ "$r" = "$exp" ] && ok "three packets decoded: DNS A? qnx.example., ICMP echo, TCP SYN" || { bad "decode"; echo "$r"; }
r=$(to 30 "$TD" -vvttnr t.pcap 2>/dev/null)
echo "$r" | has "[udp sum ok] 4660+ A? qnx.example." && echo "$r" | has "cksum 0xfb66 (correct)" \
  && echo "$r" | has "ttl 64, id 4660" && ok "-vv: IP header fields, UDP and TCP checksums verified" || { bad "-vv"; echo "$r"; }
r=$(to 30 "$TD" -ttnr t.pcap icmp 2>/dev/null)
[ "$r" = "1700000001.000000 IP 10.0.0.1 > 10.0.0.2: ICMP echo request, id 1, seq 1, length 16" ] && ok "filter 'icmp': one packet" || bad "icmp filter: $r"
r=$(to 30 "$TD" -nr t.pcap 'tcp[tcpflags] & tcp-syn != 0 and dst port 80' 2>/dev/null | wc -l | tr -d ' ')
[ "$r" = 1 ] && ok "filter on TCP flags and port" || bad "tcp flags filter: $r"
to 30 "$TD" -nr t.pcap -w f.pcap 'udp port 53' 2>/dev/null
r=$(to 30 "$TD" -ttnr f.pcap 2>/dev/null)
[ "$r" = "1700000000.000000 IP 10.0.0.1.40000 > 10.0.0.2.53: 4660+ A? qnx.example. (30)" ] && ok "-w: filtered capture written and read back" || bad "-w round trip: $r"
r=$(to 30 "$TD" -nr t.pcap -X tcp 2>/dev/null)
echo "$r" | has "0x0000:  4500 0028 1234 0000 4006 549a 0a00 0001" && echo "$r" | has "0x0020:  5002 ffff fb66 0000" \
  && ok "-X hex dump of the TCP packet" || { bad "-X"; echo "$r"; }
r=$(to 30 "$TD" -enr t.pcap -c 1 2>/dev/null)
echo "$r" | has "02:00:00:00:00:01 > 02:00:00:00:00:02, ethertype IPv4 (0x0800), length 72" && ok "-e link-level header, -c 1" || bad "-e: $r"
r=$(to 30 "$TD" -d 'tcp port 80' 2>&1)
echo "$r" | has "(000) ldh      [12]" && echo "$r" | has "ret      #0" && ok "-d: BPF program compiled" || { bad "-d"; echo "$r"; }
r=$(to 30 "$TD" -nr nonexistent.pcap 2>&1); [ $? -ne 0 ] && echo "$r" | has "nonexistent.pcap" && ok "missing file: error" || bad "missing file: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
