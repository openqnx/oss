#!/bin/sh
# CANopenNode (CANopenLinux): two canopend devices (node 1 with the CiA 309-3
# command interface on a local socket, node 4) on vcan0 of a private qcand
# (qnx-socketcan); commands go in with cocomm, frames are recorded with
# can-utils' candump.
#  - boot-up messages 701#00 and 704#00, then heartbeats;
#  - SDO client (node 1) -> SDO server (node 4): expedited upload of 0x1017
#    and 0x1000 with the exact request and response frames on the bus,
#    download + read back, abort 0x06020000 for a missing object, segmented
#    upload of the 0x1018 identity through the node's own object dictionary;
#  - NMT: "start" puts node 4 into operational (heartbeat 704#05), "preop"
#    back (704#7F), "reset communication" produces a new boot-up message;
#  - storage: 0x1010 "save" writes the parameter file; after a restart the
#    saved heartbeat time is still there;
#  - the command interface accepts a second connection after the first one
#    closed (QNX's epoll has no EPOLLONESHOT; the port re-arms the listener).
echo "== canopennode =="
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-canopennode.XXXXXX") || { echo "FAIL canopennode (no temporary directory)"; exit 1; }
pids=""
trap 'cd /; [ -n "$pids" ] && kill $pids 2>/dev/null; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
fails=0
check() { if [ "$1" = 0 ]; then echo "  ok   $2"; else echo "  FAIL $2"; fails=$((fails+1)); fi; }
has() { grep -q -- "$2" "$1"; check $? "$3"; }
QCAN_SOCKET="$T/qcand.sock"; export QCAN_SOCKET
qcand -f -s "$QCAN_SOCKET" vcan0 > qcand.log 2>&1 & qd=$!; pids="$qd"
i=0; while [ ! -S "$QCAN_SOCKET" ] && [ $i -lt 50 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$QCAN_SOCKET" ]; check $? "qcand is up with vcan0"
candump -L vcan0 > dump.txt 2>&1 & cd_=$!; pids="$pids $cd_"
sleep 0.3
mkdir n1 n4
S="$T/co.sock"
(cd n4 && exec canopend vcan0 -i 4) > n4.log 2>&1 & n4=$!
(cd n1 && exec canopend vcan0 -i 1 -c "local-$S") > n1.log 2>&1 & n1=$!
pids="$pids $n4 $n1"
i=0; while [ ! -S "$S" ] && [ $i -lt 50 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$S" ]; check $? "canopend node 1 opened its command socket"
# strip cocomm's colour escapes and line ends
co() { cocomm -s "$S" "$1" 2>&1 | sed 's/\x1b\[[0-9;]*m//g' | tr -d '\r\n'; }
waitfor() { n=0; while [ $n -lt 50 ]; do grep -q -- "$1" dump.txt && return 0; sleep 0.1; n=$((n+1)); done; return 1; }
waitfor "vcan0 701#00"; check $? "node 1 sends its boot-up message 701#00"
waitfor "vcan0 704#00"; check $? "node 4 sends its boot-up message 704#00"
has n4.log "Node ID = 0x04, running" "node 4 runs with the node id from -i"

r=$(co "4 r 0x1017 0 u16"); [ "$r" = "[1] 0" ]; check $? "SDO read 0x1017 of node 4: '$r'"
has dump.txt "vcan0 604#4017100000000000" "SDO upload request 604#40 17 10 00 is on the bus"
has dump.txt "vcan0 584#4B17100000000000" "SDO upload response 584#4B 17 10 00 (2 bytes, 0)"
r=$(co "4 r 0x1000 0 u32"); [ "$r" = "[1] 0" ]; check $? "SDO read device type 0x1000: '$r'"
r=$(co "4 w 0x1017 0 u16 500"); [ "$r" = "[1] OK" ]; check $? "SDO write 0x1017 = 500: '$r'"
has dump.txt "vcan0 604#2B171000F4010000" "SDO download request carries 0x01F4 little-endian"
has dump.txt "vcan0 584#6017100000000000" "SDO download is acknowledged (584#60)"
r=$(co "4 r 0x1017 0 u16"); [ "$r" = "[1] 500" ]; check $? "0x1017 reads back 500: '$r'"
r=$(co "4 r 0x1008 0 vs"); case "$r" in *"ERROR:0x06020000"*) e=0 ;; *) e=1 ;; esac
check $e "missing object 0x1008 is refused with abort 0x06020000: '$r'"
has dump.txt "vcan0 584#8008100000000206" "the SDO abort frame is on the bus"
r=$(co "1 r 0x1018 0 u8"); [ "$r" = "[1] 4" ]; check $? "node 1 answers for its own identity object (4 entries): '$r'"
: > mark; sleep 1.3
n=$(grep -c "vcan0 704#7F" dump.txt); [ "$n" -ge 2 ]; check $? "node 4 sends heartbeats every 500 ms in pre-operational ($n seen)"

r=$(co "4 start"); [ "$r" = "[1] OK" ]; check $? "NMT start node 4: '$r'"
has dump.txt "vcan0 000#0104" "NMT command 000#01 04 is on the bus"
r=$(co "4 preop"); [ "$r" = "[1] OK" ]; check $? "NMT enter pre-operational: '$r'"
has dump.txt "vcan0 000#8004" "NMT command 000#80 04 is on the bus"

# save the communication parameters of node 4, restart it
r=$(co "4 w 0x1010 1 vs save"); [ "$r" = "[1] OK" ]; check $? "store parameters (0x1010 sub 1 = 'save'): '$r'"
[ -s n4/od_comm.persist ]; check $? "the parameter file od_comm.persist was written"
kill $n4; wait $n4 2>/dev/null
before=$(grep -c "vcan0 704#00" dump.txt)
(cd n4 && exec canopend vcan0 -i 4) > n4b.log 2>&1 & n4=$!; pids="$pids $n4"
n=0; while [ $n -lt 50 ] && [ "$(grep -c 'vcan0 704#00' dump.txt)" -le "$before" ]; do sleep 0.1; n=$((n+1)); done
[ "$(grep -c 'vcan0 704#00' dump.txt)" -gt "$before" ]; check $? "restarted node 4 boots again"
r=$(co "4 r 0x1017 0 u16"); [ "$r" = "[1] 500" ]; check $? "the saved heartbeat time survived the restart: '$r'"
if grep -q "Emergency message from node 0x04" n4b.log; then e=1; else e=0; fi
check $e "no storage emergency after the restart with saved parameters"
waitfor "vcan0 704#05"; check $? "node 4 goes operational by itself (heartbeat 704#05)"
before=$(grep -c "vcan0 704#00" dump.txt)
r=$(co "4 reset comm"); [ "$r" = "[1] OK" ]; check $? "NMT reset communication: '$r'"
n=0; while [ $n -lt 50 ] && [ "$(grep -c 'vcan0 704#00' dump.txt)" -le "$before" ]; do sleep 0.1; n=$((n+1)); done
[ "$(grep -c 'vcan0 704#00' dump.txt)" -gt "$before" ]; check $? "reset communication produces a new boot-up message"

kill -TERM $n1 $n4; wait $n1; e1=$?; wait $n4; e4=$?
[ "$e1" = 0 ] && [ "$e4" = 0 ]; check $? "SIGTERM ends both devices with exit status 0 ($e1, $e4)"
has n1.log "finished" "node 1 logs its shutdown"
if [ $fails = 0 ]; then echo "PASS canopennode"; else echo "FAIL canopennode ($fails)"; sed 's/^/    n1: /' n1.log | tail -15; sed 's/^/    n4: /' n4.log | tail -8; tail -30 dump.txt | sed 's/^/    bus: /'; fi
[ $fails = 0 ]
