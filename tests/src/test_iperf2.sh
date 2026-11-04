#!/bin/sh
# iperf 2 on loopback: TCP and UDP servers on 127.0.0.1, then client runs
# with fixed byte counts and CSV output (-y C), whose byte fields must be
# exact: TCP -n 2M (client and server report 2097152 bytes), reverse mode
# (-R), full duplex (both directions 1048576), two parallel streams (-P 2:
# the sum line is the sum of the streams), and UDP -n 256K of 1024-byte
# datagrams (the server counts 257 datagrams = 256 + the final one, and
# 263168 bytes).
echo "== iperf2 =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-iperf2.XXXXXX) || { echo "FAIL iperf2 (mktemp)"; exit 1; }
TPID=; UPID=
# Stop the servers this test started -- on every exit path, also when the
# test is interrupted or killed by a timeout -- and remove the scratch dir.
stop_pid() {
  [ -n "$1" ] || return 0
  kill $1 2>/dev/null
  i=0; while kill -0 $1 2>/dev/null && [ $i -lt 5 ]; do sleep 1; i=$((i+1)); done
  kill -9 $1 2>/dev/null; wait $1 2>/dev/null
}
stop_servers() { stop_pid "$TPID"; TPID=; stop_pid "$UPID"; UPID=; }
cleanup() { stop_servers; cd /; rm -rf "$T"; }
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

# CSV field <n> of the lines of <file> matching <regex>.
field() { grep -E "$2" "$1" | cut -d, -f"$3"; }

V=$(iperf -v 2>&1 | head -n 1)
echo "  info $V"
case "$V" in "iperf version 2.2.1 "*) ok "iperf -v: 2.2.1" ;; *) bad "iperf -v: $V" ;; esac

PORT=$((15401 + $$ % 200))
iperf -s -B 127.0.0.1 -p $PORT -y C > tcp-server.csv 2> tcp-server.err &
TPID=$!
iperf -s -u -B 127.0.0.1 -p $((PORT+1)) -y C > udp-server.csv 2> udp-server.err &
UPID=$!
echo "  info servers on 127.0.0.1:$PORT (TCP, pid $TPID) and :$((PORT+1)) (UDP, pid $UPID)"

# The server needs a moment to listen: retry the first client a few times.
i=0; rc=1
while [ $i -lt 10 ]; do
  iperf -c 127.0.0.1 -p $PORT -n 2M -y C > tcp.csv 2> tcp.err; rc=$?
  [ $rc -eq 0 ] && [ -s tcp.csv ] && break
  i=$((i+1)); sleep 1
done
if [ $rc -eq 0 ]; then
  ok "TCP client run completed (after $i retries)"
  b=$(field tcp.csv ",127\.0\.0\.1,$PORT," 8 | head -n 1)
  [ "$b" = 2097152 ] && ok "TCP: client sent exactly 2097152 bytes" || bad "TCP: client bytes '$b' ($(head -c 200 tcp.csv))"
else
  bad "TCP client: rc=$rc $(head -c 300 tcp.err)"
fi

iperf -c 127.0.0.1 -p $PORT -n 1M -R -y C > rev.csv 2> rev.err; rc=$?
b=$(field rev.csv "^[0-9]+,127\.0\.0\.1,$PORT," 8 | head -n 1)
[ $rc -eq 0 ] && [ "$b" = 1048576 ] && ok "reverse (-R): the server sent 1048576 bytes to the client" \
  || bad "reverse: rc=$rc bytes '$b' $(head -c 300 rev.err)"

iperf -c 127.0.0.1 -p $PORT -n 1M --full-duplex -y C > fd.csv 2> fd.err; rc=$?
n=$(grep -E -c "^[0-9]+,.*,1048576,[0-9]+$" fd.csv)
[ $rc -eq 0 ] && [ "$n" -ge 2 ] && ok "full duplex: 1048576 bytes in each direction ($n lines)" \
  || bad "full duplex: rc=$rc $(cat fd.csv | head -c 400) $(head -c 200 fd.err)"

iperf -c 127.0.0.1 -p $PORT -t 1 -P 2 -y C > par.csv 2> par.err; rc=$?
s1=$(field par.csv ",127\.0\.0\.1,$PORT,1," 8 | head -n 1)
s2=$(field par.csv ",127\.0\.0\.1,$PORT,2," 8 | head -n 1)
sum=$(field par.csv ",127\.0\.0\.1,$PORT,-1," 8 | head -n 1)
if [ $rc -eq 0 ] && [ -n "$s1" ] && [ -n "$s2" ] && [ -n "$sum" ] && [ "$sum" -gt 0 ] && [ $((s1 + s2)) -eq "$sum" ]; then
  ok "parallel (-P 2): stream bytes $s1 + $s2 = sum line $sum"
else
  bad "parallel: rc=$rc s1='$s1' s2='$s2' sum='$sum' $(head -c 300 par.err)"
fi

iperf -c 127.0.0.1 -u -p $((PORT+1)) -n 256K -l 1024 -b 2M -y C > udp.csv 2> udp.err; rc=$?
b=$(field udp.csv ",127\.0\.0\.1,$((PORT+1)),1," 8 | head -n 1)
[ $rc -eq 0 ] && [ "$b" = 263168 ] && ok "UDP: client sent 263168 bytes (257 datagrams of 1024)" \
  || bad "UDP client: rc=$rc bytes '$b' $(head -c 300 udp.err)"

# The servers write their CSV reports to a file only when they exit.
stop_servers
grep -E -q "^[0-9]+,127\.0\.0\.1,[0-9]+,127\.0\.0\.1,[0-9]+,[0-9]+,[0-9.-]+,2097152," tcp-server.csv \
  && ok "TCP: server received exactly 2097152 bytes" || bad "TCP: server: $(head -c 300 tcp-server.csv)"
l=$(grep ",263168," udp-server.csv | head -n 1)
tot=$(echo "$l" | cut -d, -f12); lost=$(echo "$l" | cut -d, -f11)
echo "  info UDP server: $l"
[ "$tot" = 257 ] && ok "UDP: server counted 257 datagrams, 263168 bytes (lost $lost)" || bad "UDP server: '$l' $(head -c 200 udp-server.err)"

cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS iperf2"; exit 0; }
echo "FAIL iperf2 ($fail check(s) failed)"; exit 1
