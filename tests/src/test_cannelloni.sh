#!/bin/sh
# cannelloni: two instances join vcan0 and vcan1 of a private qcand
# (qnx-socketcan) over the loopback interface; frames are sent and recorded
# with can-utils' cansend and candump.
#  - UDP: a classic frame, an extended frame, a remote frame and a CAN FD
#    frame (64 bytes) sent on vcan0 arrive on vcan1 with id, flags and data;
#    a frame sent on vcan1 arrives on vcan0; 50 frames arrive in order (the
#    frame buffer and the timer that flushes it: QNX has no
#    timerfd_gettime(), the port keeps the timer state itself);
#  - TCP (server and client): frames cross in both directions;
#  - SIGTERM ends an instance (signalfd) with exit status 0.
echo "== cannelloni =="
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-cannelloni.XXXXXX") || { echo "FAIL cannelloni (no temporary directory)"; exit 1; }
pids=""
trap 'cd /; [ -n "$pids" ] && kill $pids 2>/dev/null; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
fails=0
check() { if [ "$1" = 0 ]; then echo "  ok   $2"; else echo "  FAIL $2"; fails=$((fails+1)); fi; }
has() { grep -q -- "$2" "$1"; check $? "$3"; }
QCAN_SOCKET="$T/qcand.sock"; export QCAN_SOCKET
qcand -f -s "$QCAN_SOCKET" vcan0 vcan1 > qcand.log 2>&1 & qd=$!; pids="$qd"
i=0; while [ ! -S "$QCAN_SOCKET" ] && [ $i -lt 50 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$QCAN_SOCKET" ]; check $? "qcand is up with vcan0 and vcan1"
P1=$((21000 + $$ % 2000)); P2=$((P1 + 1)); P3=$((P1 + 2))
# wait until a frame sent on $1 comes out on $2 (the tunnel is up)
tunnel_up() {
  n=0
  while [ $n -lt 40 ]; do
    candump -L -n 1 -T 300 "$2",7FF:7FF > probe.txt 2>/dev/null & cp=$!
    sleep 0.1; cansend "$1" 7FF#00; wait $cp
    grep -q "7FF#00" probe.txt && return 0
    n=$((n+1))
  done
  return 1
}

# --- UDP ---
cannelloni -I vcan0 -R 127.0.0.1 -r $P2 -l $P1 -t 20000 > a.log 2>&1 & a=$!
cannelloni -I vcan1 -R 127.0.0.1 -r $P1 -l $P2 -t 20000 > b.log 2>&1 & b=$!
pids="$qd $a $b"
tunnel_up vcan0 vcan1; check $? "UDP: the tunnel vcan0 -> vcan1 carries a frame"
candump -L -T 1500 vcan1 > d1.txt 2>&1 & d=$!
sleep 0.3
cansend vcan0 123#DEADBEEF
cansend vcan0 18FEF100#0102030405060708
cansend vcan0 321#R
cansend vcan0 456##1000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F
wait $d
has d1.txt "vcan1 123#DEADBEEF" "UDP: classic frame 123#DEADBEEF arrives on vcan1"
has d1.txt "vcan1 18FEF100#0102030405060708" "UDP: extended frame keeps its 29-bit id"
has d1.txt "vcan1 321#R" "UDP: remote frame arrives as a remote frame"
has d1.txt "vcan1 456##[15]000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F202122232425262728292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F" "UDP: CAN FD frame of 64 bytes arrives whole"
candump -L -T 1500 vcan0 > d0.txt 2>&1 & d=$!
sleep 0.3
cansend vcan1 2AA#CAFE
wait $d
has d0.txt "vcan0 2AA#CAFE" "UDP: frame sent on vcan1 arrives on vcan0"
n=$(grep -c "2AA#CAFE" d0.txt); [ "$n" = 1 ]; check $? "UDP: the frame is not echoed back (seen $n time(s))"
candump -L -T 2000 vcan1 > d2.txt 2>&1 & d=$!
sleep 0.3
i=0; while [ $i -lt 50 ]; do cansend vcan0 "$(printf '100#%02X' $i)"; i=$((i+1)); done
wait $d
sed -n 's/.* vcan1 100#//p' d2.txt > got.txt
i=0; : > want.txt; while [ $i -lt 50 ]; do printf '%02X\n' $i >> want.txt; i=$((i+1)); done
cmp -s got.txt want.txt; check $? "UDP: 50 frames arrive, in order ($(wc -l < got.txt | tr -d ' ') received)"
kill -TERM $a; wait $a; rc=$?
[ $rc = 0 ]; check $? "SIGTERM ends cannelloni with status 0 (got $rc)"
kill -TERM $b; wait $b
pids="$qd"

# --- TCP ---
cannelloni -C s -I vcan0 -l $P3 > s.log 2>&1 & a=$!
pids="$qd $a"; sleep 0.5
cannelloni -C c -I vcan1 -R 127.0.0.1 -r $P3 > c.log 2>&1 & b=$!
pids="$qd $a $b"
tunnel_up vcan0 vcan1; check $? "TCP: the tunnel vcan0 -> vcan1 carries a frame"
candump -L -T 1500 vcan1 > t1.txt 2>&1 & d=$!
sleep 0.3
cansend vcan0 111#1122334455667788
cansend vcan0 1ABCDEF0#99
wait $d
has t1.txt "vcan1 111#1122334455667788" "TCP: classic frame arrives on vcan1"
has t1.txt "vcan1 1ABCDEF0#99" "TCP: extended frame arrives on vcan1"
candump -L -T 1500 vcan0 > t0.txt 2>&1 & d=$!
sleep 0.3
cansend vcan1 222#0A0B0C
wait $d
has t0.txt "vcan0 222#0A0B0C" "TCP: frame sent on vcan1 arrives on vcan0"
kill -TERM $b $a; wait $a $b 2>/dev/null
pids="$qd"
if [ $fails -ne 0 ]; then for f in a.log b.log s.log c.log; do echo "--- $f"; tail -5 $f; done; fi
if [ $fails -eq 0 ]; then echo "PASS cannelloni"; exit 0; fi
echo "FAIL cannelloni ($fails)"; exit 1
