#!/bin/sh
# can-utils, the tools that work on CAN data without a CAN socket (QNX has
# no SocketCAN). Known answers, taken from the same can-utils release built
# for Linux:
#  - log2long prints a candump log (a standard, an extended, a remote, a
#    CAN FD and an error frame) in candump's long form;
#  - log2asc converts the log to Vector ASC (the frame lines), and asc2log
#    converts that back to the same frames (the CAN FD frame gaining the
#    FDF flag, as on Linux);
#  - can-calc-bit-timing: the registers for 500 kbit/s on an MCP2515 with
#    an 8 MHz clock (CNF1-3 0x00 0xb5 0x01), nominal 1 Mbit/s and data
#    5 Mbit/s on an MCP2518FD at 80 MHz (0x003a1309 / 0x000a0301), the
#    list of 37 known controllers, and an unknown controller refused.
echo "== can-utils =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-canutils.XXXXXX) || { echo "FAIL can-utils (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
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

cd /; rm -rf "$T"; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS can-utils"; exit 0; }
echo "FAIL can-utils ($fail check(s) failed)"; exit 1
