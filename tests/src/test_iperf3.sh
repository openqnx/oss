#!/bin/sh
# iperf3 on loopback: a server on 127.0.0.1, then TCP client runs with a
# fixed byte count (-n 2M, JSON output must report exactly 2097152 bytes
# sent and received), the same in reverse mode (-R), and a short UDP run.
# The port's iperf3 by path: SDP 8.0.6 images carry their own
# /system/bin/iperf3, which PATH may find first.
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
L=/usr/local; [ -x "$L/bin/iperf3" ] || L=/data/oss/usr/local
iperf3() { "$L/bin/iperf3" "$@"; }

# iperf3 always builds its own cJSON copy (src/cjson.c, ESnet-modified:
# 64-bit valueint), hence "(cJSON 1.7.15)" whatever the cjson port is.
V=$(iperf3 --version 2>&1 | head -n 1)
case "$V" in "iperf 3."[0-9]*) ok "$L/bin/iperf3 --version: $V" ;; *) bad "iperf3 --version: $V" ;; esac

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
  # field <summary> <key>: a number from the end summary (sum_sent/sum_received)
  field() { sed -n "/\"$1\":/,/}/p" udp.json | grep "\"$2\":" | head -n 1 | tr -dc '0-9'; }
  sp=$(field sum_sent packets); sb=$(field sum_sent bytes)
  rp=$(field sum_received packets); lost=$(field sum_received lost_packets)
  echo "  info UDP: sent $sp datagrams ($sb bytes), receiver counted $rp, lost_packets=$lost"
  # -n 256K of 1024-byte datagrams: at least 256. In throttled (-b) UDP
  # runs the main thread checks -n when it wakes, at each report interval
  # (3.17.1 stopped at exactly 256; 3.19 and 3.22 go on to the end of that
  # interval: 489 at 2 Mbit/s), as upstream documents -n since 3.22.
  [ -n "$sp" ] && [ "$sp" -ge 256 ] && [ "$sb" = $((sp * 1024)) ] \
    && ok "UDP: -n 256K sent $sp datagrams, every one 1024 bytes" \
    || bad "UDP: sum_sent packets $sp, bytes $sb"
  [ -n "$rp" ] && [ "$rp" = "$sp" ] && ok "UDP: the receiver accounted for all $sp datagrams" \
    || bad "UDP: sent $sp, receiver counted $rp"
else
  bad "UDP run: rc=$rc $(head -c 300 udp.err)"
fi

stop_server
echo "  info server log: $(head -n 3 server.log | tr '\n' ' ')"

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS iperf3"; exit 0; }
echo "FAIL iperf3 ($fail check(s) failed)"; exit 1
