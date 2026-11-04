#!/bin/sh
# netmap applications on io-sock's built-in netmap, with no NIC: everything
# runs on VALE software switches and netmap pipes, which io-sock creates on
# demand.
#  - /dev/netmap is there (io-sock's netmap device);
#  - pkt-gen: 1000 60-byte frames sent on one VALE port arrive, all of them
#    and no more, on another port of the same switch (1000 packets, 60000
#    bytes);
#  - pkt-gen over a netmap pipe (master {1 -> slave }1): exactly 5000
#    128-byte frames (640000 bytes); then txseq -> rxseq for two seconds:
#    packets flow and the receiver sees no sequence gap or reordering;
#  - nmreplay: a pcap file written here (50 98-byte frames) is replayed
#    in a loop into a VALE port; the receiving port counts at least 1000
#    frames, every one of them 98 bytes;
#  - opening a netmap port of an interface that does not exist fails.
# With io-sock's default netmap tunables (dev.netmap.priv_if_num=2) at most
# two netmap endpoints exist at a time, so every case uses two. Every
# process started is stopped on every exit path.
echo "== netmap =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-netmap.XXXXXX) || { echo "FAIL netmap (mktemp)"; exit 1; }
PIDS=
stop_all() {
  for p in $PIDS; do kill -INT $p 2>/dev/null; done
  for p in $PIDS; do
    i=0; while kill -0 $p 2>/dev/null && [ $i -lt 5 ]; do sleep 1; i=$((i+1)); done
    kill -9 $p 2>/dev/null; wait $p 2>/dev/null
  done
  PIDS=
}
cleanup() { stop_all; cd /; rm -rf "$T"; }
trap cleanup EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
# Switch and port names unique to this run (VALE names: "vale" + letters/digits).
SW=valeq$$
# A receiver started in the background; waits until it has opened its port.
start_rx() {  # start_rx <log> <pkt-gen args...>
  log=$1; shift
  pkt-gen "$@" > "$log" 2>&1 &
  PIDS="$PIDS $!"
  i=0; while [ $i -lt 20 ] && ! grep -qE 'reading from|aborting' "$log"; do sleep 1; i=$((i+1)); done
}
start_tx() {  # start_tx <log> <pkt-gen args...>: a sender, in the background
  log=$1; shift
  pkt-gen "$@" > "$log" 2>&1 &
  PIDS="$PIDS $!"
}
wait_done() {  # wait_done <log> <pattern>: until the line appears (max 60 s)
  i=0; while [ $i -lt 60 ] && ! grep -q "$2" "$1"; do sleep 1; i=$((i+1)); done
}

if [ -c /dev/netmap ]; then ok "/dev/netmap (io-sock netmap)"; else bad "/dev/netmap missing"; fi

# VALE switch: port a -> port b (broadcast frames are flooded to b).
start_rx rx1.log -i $SW:b -f rx -n 1000
start_tx tx1.log -i $SW:a -f tx -n 1000 -l 60
wait_done rx1.log '^Received'
wait_done tx1.log '^Sent'
grep -q 'Sent 1000 packets 60000 bytes' tx1.log && ok "VALE: sent 1000 x 60 bytes" || bad "VALE tx: $(grep -E 'Sent|memory|aborting' tx1.log)"
grep -q 'Received 1000 packets 60000 bytes' rx1.log && ok "VALE: received 1000 packets, 60000 bytes" || bad "VALE rx: $(grep -E 'Received|memory|aborting' rx1.log)"
stop_all

# netmap pipe: master {1 sends, slave }1 receives (no loss: tx waits for
# room). The sender is stopped afterwards: once the receiver has gone, the
# pipe never reports its last slots as transmitted, and pkt-gen's final
# "wait for the ring to drain" would wait for ever.
start_rx rx2.log -i $SW:p}1 -f rx -n 5000
start_tx tx2.log -i $SW:p{1 -f tx -n 5000 -l 128
wait_done rx2.log '^Received'
grep -q 'Received 5000 packets 640000 bytes' rx2.log && ok "pipe: received 5000 packets, 640000 bytes" || bad "pipe rx: $(grep -E 'Received|memory|aborting' rx2.log)"
stop_all

# Sequence-numbered traffic over the pipe for two seconds.
start_rx rx3.log -i $SW:s}1 -f rxseq
start_tx tx3.log -i $SW:s{1 -f txseq
sleep 2
stop_all
n=$(sed -n 's/^Received \([0-9]*\) packets.*/\1/p' rx3.log)
echo "  info txseq/rxseq: ${n:-no} packets in about 2 s"
[ -n "$n" ] && [ "$n" -gt 1000 ] && ok "txseq -> rxseq: $n packets" || bad "rxseq: $(grep -E 'Received|memory|aborting' rx3.log)"
grep -qE 'Sequence (GAP|OUT OF ORDER)' rx3.log && bad "rxseq: $(grep -m1 -E 'Sequence' rx3.log)" || ok "rxseq: no sequence gap or reordering"

# nmreplay: a pcap of 50 98-byte broadcast frames (49 back to back, the
# last 1 ms later), replayed in a loop into port r; port b of the same
# switch counts them. (nmreplay logs a line per pass over the trace: 50
# frames keep that short.) Built by doubling, few processes: shell loops are
# slow on an emulated target.
rec() {  # rec <microseconds byte 0> <byte 1>: one pcap record, 16 + 98 bytes
  printf "\\000\\000\\000\\000\\$1\\$2\\000\\000\\142\\000\\000\\000\\142\\000\\000\\000"
  printf '\377\377\377\377\377\377\002\000\000\000\000\001\010\000'
  printf '%84s' '' | tr ' ' '\000'
}
rec 000 000 > r1; cat r1 r1 > r2; cat r2 r2 > r4; cat r4 r4 > r8; cat r8 r8 > r16; cat r16 r16 > r32
{ printf '\324\303\262\241\002\000\004\000\000\000\000\000\000\000\000\000\377\377\000\000\001\000\000\000'
  cat r32 r16 r1; rec 350 003; } > fifty.pcap
sz=$(wc -c < fifty.pcap | tr -d ' ')
[ "$sz" = 5724 ] && ok "pcap file: 5724 bytes (24 + 50 x (16 + 98))" || bad "pcap size $sz"
start_rx rx4.log -i $SW:b -f rx -n 1000
nmreplay -f fifty.pcap -i $SW:r > rp.log 2>&1 &
PIDS="$PIDS $!"
wait_done rx4.log '^Received'
n=$(sed -n "s/^Received \([0-9]*\) packets \([0-9]*\) bytes.*/\1 \2/p" rx4.log)
set -- $n
[ -n "$1" ] && [ "$1" -ge 1000 ] && [ "$2" = $(($1 * 98)) ] && ok "nmreplay: $1 frames, all of 98 bytes" || bad "nmreplay: rx $(grep -E 'Received|memory|aborting' rx4.log); $(grep -iE 'error|fail|cannot' rp.log | head -2)"
stop_all

# A port of an interface that does not exist: refused (pkt-gen then prints
# its usage, which takes a while on an emulated target).
pkt-gen -i netmap:nosuch$$ -f rx -n 1 > m.log 2>&1 &
mp=$!; PIDS="$PIDS $mp"; wait_done m.log aborting
grep -q "nosuch$$: No such device" m.log && grep -q aborting m.log && ok "missing interface: refused (No such device)" || bad "missing interface: $(grep -v '^[[:space:]]' m.log | head -8)"
stop_all

cd /; cleanup; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS netmap"; exit 0; }
echo "FAIL netmap ($fail check(s) failed)"; exit 1
