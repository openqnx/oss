#!/bin/sh
# iperf3 on loopback: a server on 127.0.0.1, then TCP client runs with a
# fixed byte count (-n 2M, JSON output must report exactly 2097152 bytes
# sent and received), the same in reverse mode (-R), and a short UDP run.
echo "== iperf3 =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-iperf3.XXXXXX) || { echo "FAIL iperf3 (mktemp)"; exit 1; }
SPID=
# Stop the server this test started -- on every exit path, also when the
# test is interrupted or killed by a timeout -- and remove the scratch dir.
stop_server() {
  [ -n "$SPID" ] || return 0
  kill $SPID 2>/dev/null
  i=0; while kill -0 $SPID 2>/dev/null && [ $i -lt 5 ]; do sleep 1; i=$((i+1)); done
  kill -9 $SPID 2>/dev/null; wait $SPID 2>/dev/null
  SPID=
}
cleanup() { stop_server; cd /; rm -rf "$T"; }
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

V=$(iperf3 --version 2>&1 | head -n 1)
echo "  info $V"
case "$V" in "iperf 3.17.1"*) ok "iperf3 --version: 3.17.1" ;; *) bad "iperf3 --version: $V" ;; esac

PORT=$((15201 + $$ % 200))
iperf3 -s -B 127.0.0.1 -p $PORT > server.log 2>&1 &
SPID=$!
echo "  info server pid $SPID on 127.0.0.1:$PORT"

# The server needs a moment to listen: retry the first client a few times.
i=0; rc=1
while [ $i -lt 10 ]; do
  iperf3 --connect-timeout 5000 -c 127.0.0.1 -p $PORT -n 2M -J > tcp.json 2> tcp.err; rc=$?
  [ $rc -eq 0 ] && break
  i=$((i+1)); sleep 1
done
if [ $rc -eq 0 ]; then
  ok "TCP client run completed (after $i retries)"
  n=$(sed -n '/"sum_sent"/,/}/p' tcp.json | grep -E -c '"bytes":[[:space:]]*2097152,')
  [ "$n" -ge 1 ] && ok "TCP: sum_sent is exactly 2097152 bytes" || bad "TCP: sum_sent bytes: $(sed -n '/"sum_sent"/,/}/p' tcp.json | grep '"bytes"')"
  n=$(sed -n '/"sum_received"/,/}/p' tcp.json | grep -E -c '"bytes":[[:space:]]*2097152,')
  [ "$n" -ge 1 ] && ok "TCP: sum_received is exactly 2097152 bytes" || bad "TCP: sum_received bytes: $(sed -n '/"sum_received"/,/}/p' tcp.json | grep '"bytes"')"
  bps=$(sed -n '/"sum_received"/,/}/p' tcp.json | grep '"bits_per_second"' | head -n 1 | tr -dc '0-9.e+')
  echo "  info TCP loopback throughput: $bps bit/s"
  grep -q '"protocol":[[:space:]]*"TCP"' tcp.json && ok "TCP: JSON reports protocol TCP" || bad "TCP: protocol"
else
  bad "TCP client: rc=$rc $(head -c 300 tcp.err) $(head -c 300 tcp.json)"
fi

iperf3 --connect-timeout 5000 -c 127.0.0.1 -p $PORT -n 1M -R -J > rev.json 2> rev.err; rc=$?
if [ $rc -eq 0 ] && grep -q '"reverse":[[:space:]]*1' rev.json \
   && sed -n '/"sum_received"/,/}/p' rev.json | grep -E -q '"bytes":[[:space:]]*1048576,'; then
  ok "reverse mode (-R): server sent 1048576 bytes to the client"
else
  bad "reverse mode: rc=$rc $(head -c 300 rev.err)"
fi

iperf3 --connect-timeout 5000 -c 127.0.0.1 -p $PORT -u -b 2M -n 256K -l 1024 -J > udp.json 2> udp.err; rc=$?
if [ $rc -eq 0 ] && grep -q '"protocol":[[:space:]]*"UDP"' udp.json; then
  sed -n '/"end":/,$p' udp.json > udp.end
  lost=$(grep '"lost_packets"' udp.end | head -n 1 | tr -dc '0-9')
  echo "  info UDP: $(grep -c '"packets"' udp.end) packet counters in the summary, lost_packets=$lost"
  grep -E -q '"packets":[[:space:]]*256,' udp.end && ok "UDP: 256 datagrams of 1024 bytes in the summary" \
    || bad "UDP: packets $(grep '"packets"' udp.end | tr -d '\t\n ')"
else
  bad "UDP run: rc=$rc $(head -c 300 udp.err)"
fi

stop_server
echo "  info server log: $(head -n 3 server.log | tr '\n' ' ')"

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS iperf3"; exit 0; }
echo "FAIL iperf3 ($fail check(s) failed)"; exit 1
