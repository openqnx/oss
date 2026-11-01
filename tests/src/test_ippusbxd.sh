#!/bin/sh
# ippusbxd 1.34 without a USB printer (none is attached to a test target):
#  - --help prints the usage with the options, exit 0;
#  - invalid arguments are refused with the documented exit codes (a port
#    above 65535: 2; --bus-device without "<bus>:<device>": 3);
#  - started in the foreground (-d, debug, no fork) for a printer that is
#    not there (-v/-m), with DNS-SD off (-B) on the loopback interface, it
#    looks for the printer through libusb, reports that it found none (or
#    that this system's libusb has no USB access) and exits by itself,
#    without having opened its IPP port. A daemon still running after 15 s
#    is killed and counted as a failure.
echo "== ippusbxd =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-ippusbxd.XXXXXX) || { echo "FAIL ippusbxd (mktemp)"; exit 1; }
DPID=
cleanup() {
  [ -n "$DPID" ] && { kill $DPID 2>/dev/null; sleep 1; kill -9 $DPID 2>/dev/null; wait $DPID 2>/dev/null; DPID=; }
  cd /; rm -rf "$T"
}
trap cleanup EXIT
trap 'echo "  FAIL interrupted"; exit 1' INT TERM HUP
cd "$T" || exit 1
X=/usr/local/sbin/ippusbxd
[ -x $X ] || X=$(command -v ippusbxd 2>/dev/null || echo /x/ippusbxd)

out=$($X --help 2>&1); rc=$?
[ $rc -eq 0 ] && echo "$out" | grep -q '^Usage: .*ippusbxd -v <vendorid> -m <productid>' \
  && echo "$out" | grep -q -- '--no-broadcast' && ok "--help: usage and options, exit 0" || bad "--help: rc=$rc $(echo "$out" | head -n 2)"

$X -d -B -p 70000 > p.out 2>&1; rc=$?
[ $rc -eq 2 ] && grep -q 'Port number must be 65535 or less' p.out && ok "-p 70000: refused, exit 2" || bad "-p 70000: rc=$rc $(cat p.out)"
$X -d -B --bus-device 1 > b.out 2>&1; rc=$?
[ $rc -eq 3 ] && grep -q 'Bus and device must be given in the format' b.out && ok "--bus-device 1: refused, exit 3" || bad "--bus-device: rc=$rc $(cat b.out)"

# Loopback interface name: lo0 on QNX.
IF=lo0; ifconfig lo0 >/dev/null 2>&1 || IF=lo
PORT=$((47000 + $$ % 1000))
$X -d -B -i $IF -p $PORT -v 04f9 -m 0f0f > run.out 2>&1 &
DPID=$!
i=0; while kill -0 $DPID 2>/dev/null && [ $i -lt 15 ]; do sleep 1; i=$((i+1)); done
if kill -0 $DPID 2>/dev/null; then
  bad "ippusbxd still running after 15 s without a printer: $(tail -n 3 run.out)"
else
  wait $DPID; rc=$?; DPID=
  echo "  info exit status $rc after ${i}s; $(grep 'Error' run.out | head -n 2 | tr '\n' ' ')"
  if grep -q 'No printer found by that vid, pid, serial or bus, device' run.out; then
    ok "no printer 04f9:0f0f: \"No printer found\", ippusbxd exits"
  elif grep -q 'libusb init failed\|failed to get list of usb devices' run.out; then
    ok "no USB access through libusb on this system: reported, ippusbxd exits"
  else bad "no-printer run: $(tail -n 5 run.out)"; fi
  grep -q 'Error' run.out && ok "the reason is logged as an error on stderr" || bad "no error logged"
fi
# It must not have left a listener on its port.
if command -v socat >/dev/null 2>&1; then
  socat -T1 /dev/null TCP:127.0.0.1:$PORT 2>/dev/null && bad "something listens on 127.0.0.1:$PORT" || ok "no IPP listener left on 127.0.0.1:$PORT"
fi

[ $fail -eq 0 ] && { echo "PASS ippusbxd"; exit 0; } || { echo "FAIL ippusbxd ($fail check(s) failed)"; exit 1; }
