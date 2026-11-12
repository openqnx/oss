#!/bin/sh
# python3-can (python-can), with python3-wrapt underneath (python-can's
# deprecation helpers import it).
#  - Message: fields, equality, DLC of CAN FD frames (dlc 15 = 64 bytes);
#  - the virtual interface: two buses of one channel exchange a frame;
#  - log files with known contents: CanutilsLogWriter writes candump's
#    format ("(1700000000.250000) vcan0 123#DEADBEEF", the extended, remote
#    and CAN FD forms) and CanutilsLogReader reads the same messages back;
#    ASC and BLF round trip the frames;
#  - the socketcan interface on QNX (qnx-socketcan through
#    can/interfaces/socketcan/qnx.py), on a private qcand with vcan0 (CAN FD)
#    and vcan1 (classic): without qcand can.Bus fails with an OSError; then
#    a frame sent on one bus arrives on another bus of the same interface
#    with id, flags, data and channel, and not on the other interface; a
#    receive time within a second of now; the sender does not see its own
#    frame unless receive_own_messages is set; an extended, a remote and a
#    CAN FD frame (64 bytes, BRS) arrive as sent; a 64-byte frame is refused
#    on the classic bus; filters (id/mask, extended) pass the matching
#    frames only; recv(timeout) returns None on a silent bus within the
#    timeout; send_periodic sends about 10 frames in 0.5 s at 50 ms; a
#    Notifier delivers frames to a listener; detect_available_configs lists
#    vcan0 and vcan1; an unknown interface name raises;
#  - can.logger records frames from the bus into a candump log file.
echo "== python3-can =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pycan.XXXXXX") || { echo "FAIL python3-can (no temporary directory)"; exit 1; }
qd=""
trap 'cd /; [ -n "$qd" ] && kill $qd 2>/dev/null; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
QCAN_SOCKET="$T/qcand.sock"; export QCAN_SOCKET
cat > t.py <<'P'
import sys, os, time, subprocess, threading
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok); sys.stdout.flush()
def info(s): print("  info " + s)
import can, wrapt
info("python-can %s, wrapt %s" % (can.__version__, wrapt.__version__))
M = can.Message
m = M(arbitration_id=0x123, data=bytes.fromhex("deadbeef"), is_extended_id=False)
check(m.dlc == 4 and m.arbitration_id == 0x123 and not m.is_extended_id and m.equals(M(arbitration_id=0x123, data=[0xde, 0xad, 0xbe, 0xef], is_extended_id=False)), "Message: id, dlc, data, equals()")
f = M(arbitration_id=0x7ff, data=bytes(range(64)), is_fd=True, bitrate_switch=True, is_extended_id=False)
check(f.dlc == 64 and can.util.len2dlc(64) == 15 and can.util.dlc2len(13) == 32, "CAN FD: 64 data bytes are DLC 15, DLC 13 is 32 bytes")

# virtual interface
a = can.Bus("t", interface="virtual"); b = can.Bus("t", interface="virtual")
a.send(m); r = b.recv(1)
check(r is not None and r.arbitration_id == 0x123 and bytes(r.data) == bytes.fromhex("deadbeef"), "virtual interface: a frame crosses between two buses")
a.shutdown(); b.shutdown()

# log files
msgs = [
    M(timestamp=1700000000.25, channel="vcan0", arbitration_id=0x123, data=bytes.fromhex("deadbeef"), is_extended_id=False),
    M(timestamp=1700000001.5, channel="vcan0", arbitration_id=0x1abcdef0, data=bytes(range(1, 9)), is_extended_id=True),
    M(timestamp=1700000002.0, channel="vcan0", arbitration_id=0x456, is_remote_frame=True, dlc=0, is_extended_id=False),
    M(timestamp=1700000003.0, channel="vcan0", arbitration_id=0x7ff, data=bytes(range(0x11, 0x1d)), is_fd=True, bitrate_switch=True, is_extended_id=False),
]
with can.CanutilsLogWriter("out.log") as w:
    for x in msgs: w.on_message_received(x)
lines = open("out.log").read().split("\n")
want = ["(1700000000.250000) vcan0 123#DEADBEEF", "(1700000001.500000) vcan0 1ABCDEF0#0102030405060708",
        "(1700000002.000000) vcan0 456#R", "(1700000003.000000) vcan0 7FF##11112131415161718191A1B1C"]
got = [l.rsplit(" ", 1)[0] if l.endswith(" R") or l.endswith(" T") else l for l in lines if l]
check(got == want, "CanutilsLogWriter writes candump's log format" + ("" if got == want else ": %r" % got))
back = list(can.CanutilsLogReader("out.log"))
check(len(back) == 4 and all(x.arbitration_id == y.arbitration_id and bytes(x.data) == bytes(y.data) and x.is_extended_id == y.is_extended_id and x.is_remote_frame == y.is_remote_frame and x.is_fd == y.is_fd and abs(x.timestamp - y.timestamp) < 1e-6 for x, y in zip(back, msgs)), "CanutilsLogReader reads the same four messages back")
for name in ("t.asc", "t.blf"):
    with can.Logger(name) as w:
        for x in msgs[:3]: w.on_message_received(x)
    back = list(can.LogReader(name))
    check([(x.arbitration_id, bytes(x.data), x.is_extended_id, x.is_remote_frame) for x in back] == [(x.arbitration_id, bytes(x.data), x.is_extended_id, x.is_remote_frame) for x in msgs[:3]], "%s log round trips three frames" % name.split(".")[1].upper())

# socketcan interface
try:
    can.Bus("vcan0", interface="socketcan"); check(False, "socketcan without qcand fails")
except OSError as e:
    check(True, "socketcan without qcand fails (%s)" % e)
qd = subprocess.Popen(["qcand", "-f", "-s", os.environ["QCAN_SOCKET"], "vcan0", "vcan1:classic"], stdout=open("qcand.out", "w"), stderr=subprocess.STDOUT)
for i in range(50):
    if os.path.exists(os.environ["QCAN_SOCKET"]): break
    time.sleep(0.1)
try:
    tx = can.Bus("vcan0", interface="socketcan", fd=True)
    rx = can.Bus("vcan0", interface="socketcan", fd=True)
    other = can.Bus("vcan1", interface="socketcan")
    check(type(tx).__name__ == "SocketcanBus" and tx.fileno() >= 0, "can.Bus('vcan0', interface='socketcan') opens (SocketcanBus, fd %d)" % tx.fileno())
    t0 = time.time(); tx.send(m); r = rx.recv(2)
    check(r is not None and r.arbitration_id == 0x123 and bytes(r.data) == bytes.fromhex("deadbeef") and not r.is_extended_id and not r.is_fd and r.channel == "vcan0", "a frame sent on vcan0 arrives on another vcan0 bus: " + str(r))
    check(r is not None and abs(r.timestamp - t0) < 1.0, "its receive time is now (%.3f s off)" % ((r.timestamp - t0) if r else -1))
    check(other.recv(0.2) is None, "nothing arrives on vcan1")
    check(tx.recv(0.2) is None, "the sender does not receive its own frame")
    own = can.Bus("vcan0", interface="socketcan", receive_own_messages=True)
    own.send(M(arbitration_id=0x55, data=[1], is_extended_id=False)); r = own.recv(1)
    check(r is not None and r.arbitration_id == 0x55, "receive_own_messages=True: the sender receives it")
    rx.recv(0.5); own.shutdown()
    e = M(arbitration_id=0x1abcdef0, data=bytes(range(8)), is_extended_id=True); tx.send(e); r = rx.recv(1)
    check(r is not None and r.is_extended_id and r.arbitration_id == 0x1abcdef0 and bytes(r.data) == bytes(range(8)), "an extended frame arrives as sent")
    tx.send(M(arbitration_id=0x456, is_remote_frame=True, dlc=3, is_extended_id=False)); r = rx.recv(1)
    check(r is not None and r.is_remote_frame and r.dlc == 3 and r.arbitration_id == 0x456, "a remote frame arrives with its DLC")
    tx.send(f); r = rx.recv(1)
    check(r is not None and r.is_fd and r.bitrate_switch and r.dlc == 64 and bytes(r.data) == bytes(range(64)), "a CAN FD frame with 64 bytes and BRS arrives as sent")
    o2 = can.Bus("vcan1", interface="socketcan", fd=True)
    try:
        o2.send(f); refused = other.recv(0.3) is None
    except can.CanError:
        refused = True
    check(refused, "a CAN FD frame does not pass the classic bus vcan1")
    o2.shutdown()
    flt = can.Bus("vcan0", interface="socketcan", can_filters=[{"can_id": 0x100, "can_mask": 0x7f0, "extended": False}])
    for i in (0x0ff, 0x100, 0x10f, 0x110): tx.send(M(arbitration_id=i, is_extended_id=False))
    tx.send(M(arbitration_id=0x105, is_extended_id=True))
    ids = []
    while True:
        r = flt.recv(0.3)
        if r is None: break
        ids.append((r.arbitration_id, r.is_extended_id))
    check(ids == [(0x100, False), (0x10f, False)], "filter 100/7F0 standard passes 100 and 10F only: %r" % ids)
    flt.set_filters([{"can_id": 0x105, "can_mask": 0x1fffffff, "extended": True}])
    tx.send(M(arbitration_id=0x105, is_extended_id=False)); tx.send(M(arbitration_id=0x105, is_extended_id=True)); r = flt.recv(0.5)
    check(r is not None and r.is_extended_id and flt.recv(0.2) is None, "set_filters: the extended 105 passes, the standard 105 does not")
    flt.shutdown()
    while rx.recv(0.1) is not None: pass
    t0 = time.time(); r = rx.recv(0.3); dt = time.time() - t0
    check(r is None and 0.25 <= dt < 1.0, "recv(0.3) on a silent bus returns None after %.2f s" % dt)
    task = tx.send_periodic(M(arbitration_id=0x321, data=[7], is_extended_id=False), 0.05)
    time.sleep(0.5); task.stop(); n = 0
    while rx.recv(0.15) is not None: n += 1
    check(7 <= n <= 13, "send_periodic at 50 ms for 0.5 s sends about 10 frames (%d)" % n)
    seen = []; ev = threading.Event()
    class L(can.Listener):
        def on_message_received(self, msg):
            seen.append(msg.arbitration_id)
            if len(seen) == 5: ev.set()
    nt = can.Notifier(rx, [L()])
    for i in range(5): tx.send(M(arbitration_id=0x200 + i, is_extended_id=False))
    ev.wait(2); nt.stop()
    check(seen == [0x200 + i for i in range(5)], "a Notifier delivers five frames to a listener in order")
    names = sorted(c["channel"] for c in can.detect_available_configs("socketcan"))
    check(names == ["vcan0", "vcan1"], "detect_available_configs('socketcan') lists vcan0 and vcan1: %r" % names)
    try:
        can.Bus("nocan9", interface="socketcan"); check(False, "an unknown interface raises")
    except OSError as e:
        check(True, "an unknown interface raises OSError (%s)" % e)
    # can.logger
    lg = subprocess.Popen([sys.executable, "-u", "-m", "can.logger", "-i", "socketcan", "-c", "vcan0", "-f", "rec.log"], stdout=open("logger.out", "w"), stderr=open("logger.err", "w"))
    # The logger prints "Connected to ..." when its bus is open (an interpreter
    # start takes seconds on an emulated target).
    t0 = time.time()
    while time.time() - t0 < 90 and "Connected" not in open("logger.out").read(): time.sleep(0.2)
    time.sleep(0.5)
    for i in range(3): tx.send(M(arbitration_id=0x700 + i, data=[i, i], is_extended_id=False))
    time.sleep(1.0); lg.send_signal(2)
    try: lg.wait(10)
    except subprocess.TimeoutExpired: lg.kill()
    rec = [l.split(" ", 1)[1].rsplit(" ", 1)[0] if l.endswith((" R", " T")) else l.split(" ", 1)[1] for l in open("rec.log").read().split("\n") if l] if os.path.exists("rec.log") else []
    check(rec == ["vcan0 700#0000", "vcan0 701#0101", "vcan0 702#0202"], "can.logger records three frames in candump format: %r %s" % (rec, open("logger.err").read()[-300:] if rec != ["vcan0 700#0000", "vcan0 701#0101", "vcan0 702#0202"] else ""))
    tx.shutdown(); rx.shutdown(); other.shutdown()
finally:
    qd.terminate()
print("FAILS %d" % fails)
P
"$PY" t.py > out.txt 2>&1; rc=$?
cat out.txt
if [ $rc -eq 0 ] && grep -q "^FAILS 0$" out.txt; then echo "PASS python3-can"; exit 0; fi
echo "FAIL python3-can (rc=$rc)"; exit 1
