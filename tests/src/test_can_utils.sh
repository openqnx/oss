#!/bin/sh
# can-utils. First the tools that work on CAN data without a CAN socket.
# Known answers, taken from the same can-utils release built for Linux:
#  - log2long prints a candump log (a standard, an extended, a remote, a
#    CAN FD and an error frame) in candump's long form;
#  - log2asc converts the log to Vector ASC (the frame lines), and asc2log
#    converts that back to the same frames (the CAN FD frame gaining the
#    FDF flag, as on Linux);
#  - can-calc-bit-timing: the registers for 500 kbit/s on an MCP2515 with
#    an 8 MHz clock (CNF1-3 0x00 0xb5 0x01), nominal 1 Mbit/s and data
#    5 Mbit/s on an MCP2518FD at 80 MHz (0x003a1309 / 0x000a0301), the
#    list of 37 known controllers, and an unknown controller refused.
# Then the socket tools, on qnx-socketcan's CAN sockets and a private qcand
# with the buses vcan0 (CAN FD) and vcan1 (classic CAN):
#  - cansend without a daemon fails; with it, candump -L prints a standard,
#    an extended, a remote and a CAN FD frame as cansend sent them, and
#    nothing of the other bus; a CAN FD frame is refused on the classic bus;
#  - candump's id:mask filters on two buses pass the matching frames only;
#  - cangen -n 20 -I 42 -L 2 -D i sends 20 frames counting 0000 .. 1300;
#    canplayer replays candump's log of them on the other bus, in order;
#  - cansequence: 50 counted frames, checked by its receiver.
echo "== can-utils =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-canutils.XXXXXX) || { echo "FAIL can-utils (mktemp)"; exit 1; }
qd=""
trap 'cd /; [ -n "$qd" ] && kill $qd 2>/dev/null; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
has() { grep -qxF "$2" "$1"; }   # has <file> <exact line>

cat > in.log <<'EOF'
(1700000000.000000) can0 123#DEADBEEF
(1700000000.010000) can0 1ABCDEF0#0102030405060708
(1700000000.020000) can0 456#R
(1700000000.030000) can1 7FF##1112233445566778899AABBCC
(1700000000.040000) can0 20000080#0000000000000000
EOF

log2long < in.log > long.txt 2>&1
n=0
has long.txt "(1700000000.000000)  can0       123   [4]  DE AD BE EF               '....'" && n=$((n+1))
has long.txt "(1700000000.010000)  can0  1ABCDEF0   [8]  01 02 03 04 05 06 07 08   '........'" && n=$((n+1))
has long.txt "(1700000000.020000)  can0       456   [0]  remote request" && n=$((n+1))
has long.txt "(1700000000.030000)  can1       7FF  [12]  11 22 33 44 55 66 77 88 99 AA BB CC" && n=$((n+1))
has long.txt "(1700000000.040000)  can0  20000080   [8]  00 00 00 00 00 00 00 00   ERRORFRAME" && n=$((n+1))
[ $n -eq 5 ] && ok "log2long: 5 frames in long form (standard, extended, remote, FD, error)" || bad "log2long: $n/5 lines: $(head -5 long.txt)"

log2asc -I in.log -O out.asc can0 can1 > l2a.log 2>&1; rc=$?
n=0
has out.asc "   0.000000 1  123             Rx   d 4 DE AD BE EF" && n=$((n+1))
has out.asc "   0.010000 1  1ABCDEF0x       Rx   d 8 01 02 03 04 05 06 07 08" && n=$((n+1))
has out.asc "   0.020000 1  456             Rx   r 0" && n=$((n+1))
has out.asc "   0.030000 CANFD   2 Rx          7FF                                   1 0 9 12 11 22 33 44 55 66 77 88 99 AA BB CC   130000  130     3000 0 0 0 0 0" && n=$((n+1))
has out.asc "   0.040000 1  ErrorFrame" && n=$((n+1))
[ $rc -eq 0 ] && [ $n -eq 5 ] && ok "log2asc: 5 frames in Vector ASC" || bad "log2asc rc=$rc, $n/5 lines: $(cat l2a.log)"
grep -q '^base hex  timestamps absolute' out.asc && ok "log2asc: ASC header (hex, absolute timestamps)" || bad "log2asc header: $(head -3 out.asc)"

# Back to a candump log: the timestamps are rebased (the ASC file's date is
# local), so compare the frames.
asc2log -I out.asc -O back.log > a2l.log 2>&1; rc=$?
sed 's/^([0-9.]*) //' back.log > frames.txt
n=0
has frames.txt "can0 123#DEADBEEF R" && n=$((n+1))
has frames.txt "can0 1ABCDEF0#0102030405060708 R" && n=$((n+1))
has frames.txt "can0 456#R R" && n=$((n+1))
has frames.txt "can1 7FF##5112233445566778899AABBCC R" && n=$((n+1))
has frames.txt "can0 20000080#0000000000000000" && n=$((n+1))
[ $rc -eq 0 ] && [ $n -eq 5 ] && ok "asc2log: the 5 frames back" || bad "asc2log rc=$rc, $n/5: $(cat frames.txt | head -5)"
d=$(sed -n 's/^(\([0-9]*\)\.\([0-9]*\)).*/\1\2/p' back.log | awk 'NR==1{f=$1} END{print $1-f}')
[ "$d" = 40000 ] && ok "asc2log: first to last frame 40 ms apart" || bad "asc2log timestamps: span '$d' us"

can-calc-bit-timing -c 8000000 -b 500000 mcp251x > bt1.txt 2>&1
has bt1.txt "  500000    125   6    7    2   1   1   500000   0.0%  87.5%  87.5%   0.0%   0x00 0xb5 0x01" \
  && ok "bit timing: MCP2515, 8 MHz, 500 kbit/s -> CNF 0x00 0xb5 0x01, sample point 87.5%" || bad "bit timing mcp251x: $(cat bt1.txt)"
can-calc-bit-timing -q -c 80000000 -b 1000000 -d 5000000 mcp251xfd > bt2.txt 2>&1
has bt2.txt " 1000000     12  29   30   20  10   1  1000000   0.0%  75.0%  75.0%   0.0%   0x003a1309" && \
has bt2.txt " 5000000     12   5    6    4   2   1  5000000   0.0%  75.0%  75.0%   0.0%   0x000a0301" \
  && ok "bit timing: MCP2518FD, 80 MHz, 1 Mbit/s + 5 Mbit/s data -> 0x003a1309 / 0x000a0301" || bad "bit timing mcp251xfd: $(cat bt2.txt)"
n=$(can-calc-bit-timing -l 2>&1 | grep -c .)
[ "$n" = 37 ] && ok "bit timing: 37 known controllers" || bad "bit timing -l: $n"
can-calc-bit-timing -b 500000 nosuchctl > bt3.txt 2>&1; rc=$?
[ $rc -ne 0 ] && grep -q "unknown CAN controller 'nosuchctl'" bt3.txt && ok "bit timing: unknown controller refused" || bad "unknown controller: rc=$rc"

# The socket tools, on qnx-socketcan: a private qcand with two buses
# (vcan0 CAN FD, vcan1 classic CAN), found through QCAN_SOCKET.
QCAN_SOCKET="$T/qcand.sock"; export QCAN_SOCKET
cansend vcan0 123#11 > nod.txt 2>&1; rc=$?
[ $rc -ne 0 ] && grep -q "socket" nod.txt && ok "cansend without qcand fails (rc=$rc: $(head -1 nod.txt))" || bad "cansend without qcand: rc=$rc $(cat nod.txt)"
qcand -f -s "$QCAN_SOCKET" vcan0 vcan1:classic > qcand.out 2>&1 &
qd=$!
i=0; while [ ! -S "$QCAN_SOCKET" ] && [ $i -lt 50 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$QCAN_SOCKET" ] && ok "qcand is up" || bad "qcand did not start: $(cat qcand.out)"
# Wait until a background candump has its sockets bound: it prints nothing
# until a frame arrives, so probe with a frame on an ID the checks ignore.
wait_dump() { # <file> <bus>
  i=0; while [ $i -lt 50 ]; do cansend "$2" 7E0#00 2>/dev/null; sleep 0.1; grep -q "7E0#00" "$1" && return 0; i=$((i+1)); done; return 1
}
candump -L vcan0 > dump1.txt 2>&1 &
cd1=$!
wait_dump dump1.txt vcan0 && ok "candump receives on vcan0" || bad "candump saw no frame: $(cat dump1.txt)"
cansend vcan0 123#DEADBEEF; r1=$?
cansend vcan0 1ABCDEF0#0102030405060708; r2=$?
cansend vcan0 456#R; r3=$?
cansend vcan0 7FF##5112233445566778899AABBCC; r4=$?
cansend vcan1 321#CAFE; r5=$?
sleep 0.5; kill $cd1 2>/dev/null; wait $cd1 2>/dev/null
[ "$r1$r2$r3$r4$r5" = 00000 ] && ok "cansend: classic, extended, remote and CAN FD frames sent" || bad "cansend rc: $r1 $r2 $r3 $r4 $r5"
n=0
grep -q " vcan0 123#DEADBEEF\$" dump1.txt && n=$((n+1))
grep -q " vcan0 1ABCDEF0#0102030405060708\$" dump1.txt && n=$((n+1))
grep -q " vcan0 456#R\$" dump1.txt && n=$((n+1))
grep -q " vcan0 7FF##5112233445566778899AABBCC\$" dump1.txt && n=$((n+1))
[ $n -eq 4 ] && ok "candump -L: the 4 frames, as sent" || bad "candump: $n/4: $(cat dump1.txt)"
grep -q "321#CAFE" dump1.txt && bad "candump vcan0 saw a vcan1 frame" || ok "candump vcan0 does not see vcan1 frames"
cansend vcan1 7FF##5112233445566778899AABBCC > fd1.txt 2>&1; rc=$?
[ $rc -ne 0 ] && ok "cansend: a CAN FD frame is refused on the classic bus vcan1" || bad "CAN FD frame on classic bus: rc=$rc"
# Filters (id:mask), both buses in one candump, -n ends it after 3 frames
# (the probe and the two that pass).
candump -L -n 3 vcan0,300:700 vcan1,321:7FF > dump2.txt 2>&1 &
cd2=$!
i=0; while [ $i -lt 50 ]; do cansend vcan0 3E0#00 2>/dev/null; sleep 0.1; grep -q "3E0#00" dump2.txt && break; i=$((i+1)); done
cansend vcan0 245#01; cansend vcan0 345#02; cansend vcan1 322#03; cansend vcan1 321#04
i=0; while kill -0 $cd2 2>/dev/null && [ $i -lt 50 ]; do sleep 0.1; i=$((i+1)); done
kill $cd2 2>/dev/null; wait $cd2 2>/dev/null
f=$(grep -v "3E0#00" dump2.txt | sed 's/^([0-9.]*) //' | tr '\n' ' ')
[ "$f" = "vcan0 345#02 vcan1 321#04 " ] && ok "candump filters: 300:700 on vcan0 and 321:7FF on vcan1 pass 345 and 321 only" || bad "filters: '$f'"
# cangen: 20 frames, fixed ID and length, counting payload; canplayer replays them.
candump -L vcan0 > dump3.txt 2>&1 &
cd3=$!
wait_dump dump3.txt vcan0
cangen vcan0 -n 20 -g 2 -I 42 -L 2 -D i; rc=$?
sleep 0.5; kill $cd3 2>/dev/null; wait $cd3 2>/dev/null
grep " 042#" dump3.txt > gen.log
n=$(grep -c . gen.log)
[ $rc -eq 0 ] && [ "$n" = 20 ] && ok "cangen: 20 frames on ID 042 received" || bad "cangen rc=$rc, $n/20 frames"
p=$(sed 's/.*#//' gen.log | tr '\n' ' ')
[ "$p" = "0000 0100 0200 0300 0400 0500 0600 0700 0800 0900 0A00 0B00 0C00 0D00 0E00 0F00 1000 1100 1200 1300 " ] && ok "cangen -D i: the payload counts 0000 .. 1300 in order" || bad "cangen payloads: $p"
candump -L vcan1 > dump4.txt 2>&1 &
cd4=$!
wait_dump dump4.txt vcan1
canplayer -I gen.log vcan1=vcan0 > play.txt 2>&1; rc=$?
sleep 0.5; kill $cd4 2>/dev/null; wait $cd4 2>/dev/null
a=$(sed 's/.*#//' gen.log | tr '\n' ' '); b=$(grep " 042#" dump4.txt | sed 's/.*#//' | tr '\n' ' ')
[ $rc -eq 0 ] && [ "$a" = "$b" ] && [ "$(grep -c 'vcan1 042#' dump4.txt)" = 20 ] && ok "canplayer: the log replayed on vcan1, 20 frames in order" || bad "canplayer rc=$rc: $(cat play.txt | head -3) got '$b'"
# cansequence: the receiver checks the sender's counter and exits 0 after -l frames.
cansequence -r --loop=50 vcan0 > seqr.txt 2>&1 &
sq=$!
sleep 0.5
cansequence --loop=50 vcan0 > seqs.txt 2>&1; rs=$?
i=0; while kill -0 $sq 2>/dev/null && [ $i -lt 50 ]; do sleep 0.1; i=$((i+1)); done
if kill -0 $sq 2>/dev/null; then kill $sq; wait $sq 2>/dev/null; bad "cansequence receiver did not finish: $(tail -2 seqr.txt)"
else wait $sq; rr=$?; [ $rs -eq 0 ] && [ $rr -eq 0 ] && ok "cansequence: 50 frames sent and received in sequence" || bad "cansequence send rc=$rs, receive rc=$rr: $(tail -2 seqr.txt)"; fi
kill $qd 2>/dev/null; wait $qd 2>/dev/null

cd /; rm -rf "$T"; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS can-utils"; exit 0; }
echo "FAIL can-utils ($fail check(s) failed)"; exit 1
