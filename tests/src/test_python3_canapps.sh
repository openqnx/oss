#!/bin/sh
# The Python applications and tools above the CAN libraries, with known answers:
#  - pint: unit conversion (100 km/h is 62.137 mph, 3 m + 4 cm is 3040 mm),
#    dimension errors, the pint-convert command;
#  - python-OBD (obd): a small ELM327 adapter emulated on a pseudo terminal
#    answers the AT commands and mode 01 PIDs: the connection reaches "Car
#    Connected" with protocol ISO 15765-4 (CAN 11/500), the supported PID
#    bitmap is decoded, engine speed, vehicle speed, coolant temperature and
#    the adapter voltage come back as pint quantities, an unsupported PID
#    gives a null response;
#  - scapy: CAN, extended and CAN FD frames build to the expected bytes and
#    dissect back; a candump log is read and written to a pcap file that
#    reads back as CAN frames; UDS and ISO-TP layers build, fragment and
#    dissect; scapy's CANSocket (through python-can's socketcan interface,
#    qnx-socketcan underneath) sends and receives on a private qcand bus;
#    an ISOTPSoftSocket pair carries a UDS ReadDataByIdentifier request and
#    a segmented 20-byte response;
#    on the network: the interface list and the routing table are read from
#    io-sock (PF_ROUTE), an ICMP echo request sent through /dev/bpf on lo0
#    is answered (sr1) and a sniffer with a compiled capture filter (libpcap)
#    sees both packets.
# scapy's native Linux CAN sockets and the CAN_ISOTP kernel sockets are not
# there on QNX; the python-can and soft-socket variants are the ones to use.
echo "== python3-canapps =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pycanapps.XXXXXX") || { echo "FAIL python3-canapps (no temporary directory)"; exit 1; }
trap 'cd /; [ -f "$T/qcand.pid" ] && kill "$(cat "$T/qcand.pid")" 2>/dev/null; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
QCAN_SOCKET="$T/qcand.sock"; export QCAN_SOCKET
HOME="$T"; export HOME
cat > t.py <<'P'
import sys, os, time, subprocess, threading
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok); sys.stdout.flush()
def hx(b): return bytes(b).hex()

# ---- pint
import pint
u = pint.UnitRegistry()
check(abs((100 * u.kph).to(u.mph).magnitude - 62.1371) < 1e-3, "pint: 100 km/h is 62.137 mph")
check(str((3 * u.meter + 4 * u.cm).to(u.mm)) == "3040.0 millimeter", "pint: 3 m + 4 cm is 3040.0 millimeter")
try: (1 * u.meter).to(u.second); ok = False
except pint.DimensionalityError: ok = True
check(ok, "pint: metre to second raises DimensionalityError")
check(abs(u.Quantity(25, u.degC).to(u.kelvin).magnitude - 298.15) < 1e-9, "pint: 25 degC is 298.15 K")
r = subprocess.run([os.path.join(os.path.dirname(sys.executable), "pint-convert"), "100 kph", "mph"], capture_output=True, text=True)
check(r.returncode == 0 and "62.137" in r.stdout, "pint-convert 100 kph mph prints 62.137...: %r" % r.stdout.strip()[:60])

# ---- python-OBD against an ELM327 on a pseudo terminal
import obd
obd.logger.setLevel(obd.logging.CRITICAL)
m = None
for n in range(4, 16):
    try: m = os.open("/dev/ptyp%x" % n, os.O_RDWR); tty = "/dev/ttyp%x" % n; break
    except OSError: pass
if m is None:
    check(False, "obd: no free pseudo terminal")
else:
    log = []
    R = {"ATZ": "ELM327 v1.5", "ATE0": "OK", "ATH1": "OK", "ATL0": "OK", "ATRV": "12.5V", "ATSP0": "OK", "ATDPN": "A6",
         "0100": "7E8 06 41 00 BE 3F A8 13", "010C": "7E8 04 41 0C 2E E0", "010D": "7E8 03 41 0D 64", "0105": "7E8 03 41 05 7B",
         "0120": "7E8 06 41 20 00 00 00 00", "0600": "NO DATA", "0900": "NO DATA"}
    def elm():
        buf = b""
        while True:
            try: d = os.read(m, 256)
            except OSError: return
            if not d: return
            buf += d
            while b"\r" in buf:
                line, buf = buf.split(b"\r", 1)
                c = line.decode().replace(" ", "").upper(); log.append(c)
                os.write(m, (R.get(c, "?") + "\r\r>").encode())
    threading.Thread(target=elm, daemon=True).start()
    c = obd.OBD(tty, baudrate=38400, fast=False, timeout=1)
    check(c.status() == obd.OBDStatus.CAR_CONNECTED and c.is_connected(), "obd: connection on %s reaches %r" % (tty, c.status()))
    check(c.protocol_id() == "6" and c.protocol_name() == "ISO 15765-4 (CAN 11/500)", "obd: protocol 6, ISO 15765-4 (CAN 11/500)")
    sup = set(x.name for x in c.supported_commands)
    check({"RPM", "SPEED", "COOLANT_TEMP"} <= sup and "FUEL_LEVEL" not in sup, "obd: the supported PID bitmap is decoded (%d commands)" % len(sup))
    r = c.query(obd.commands.RPM)
    check(not r.is_null() and r.value.magnitude == 3000.0 and str(r.value.units) == "revolutions_per_minute", "obd: RPM 2E E0 is 3000 rpm")
    r = c.query(obd.commands.SPEED)
    check(r.value.magnitude == 100 and abs(r.value.to("mph").magnitude - 62.137) < 1e-3, "obd: SPEED 0x64 is 100 km/h, 62.137 mph")
    r = c.query(obd.commands.COOLANT_TEMP)
    check(r.value.magnitude == 83, "obd: COOLANT_TEMP 0x7B is 83 degC")
    r = c.query(obd.commands.ELM_VOLTAGE)
    check(r.value.magnitude == 12.5, "obd: adapter voltage 12.5 V")
    check(c.query(obd.commands.FUEL_LEVEL).is_null(), "obd: an unsupported PID gives a null response")
    c.close()
    check(log[:6] == ["ATZ", "ATE0", "ATH1", "ATL0", "ATRV", "ATSP0"] and "010C" in log, "obd: the adapter saw ATZ ATE0 ATH1 ATL0 ATRV ATSP0 ... 010C")
    os.close(m)

# ---- scapy: CAN layers
import logging; logging.getLogger("scapy").setLevel(logging.ERROR)
from scapy.all import *
conf.verb = 0
conf.contribs['CANSocket'] = {'use-python-can': True}
conf.contribs['ISOTP'] = {'use-can-isotp-kernel-module': False}
load_layer("can"); load_contrib("cansocket"); load_contrib("isotp"); load_contrib("automotive.uds")
f = CAN(identifier=0x123, length=3, data=b"\x01\x02\x03")
check(hx(f) == "0000012303000000010203" and CAN(bytes(f)).identifier == 0x123 and CAN(bytes(f)).data == b"\x01\x02\x03", "scapy: CAN 123#010203 builds and dissects")
e = CAN(identifier=0x18FECA80, flags="extended", data=b"\xaa" * 8)
check(hx(e) == "98feca8008000000" + "aa" * 8 and str(CAN(bytes(e)).flags) == "extended", "scapy: extended frame 18FECA80 (flag bit 31)")
fd = CANFD(identifier=0x7ff, length=12, data=bytes(range(12)), fd_flags="bit_rate_switch")
check(hx(fd) == "000007ff0c010000" + bytes(range(12)).hex(), "scapy: CAN FD frame with 12 bytes and BRS")
open("c.log", "w").write("(1700000000.100000) vcan0 123#010203\n(1700000000.200000) vcan0 18FECA80#AAAAAAAAAAAAAAAA\n")
p = rdcandump("c.log")
check(len(p) == 2 and p[0].identifier == 0x123 and str(p[1].flags) == "extended" and abs(float(p[1].time) - 1700000000.2) < 1e-3, "scapy: rdcandump reads two frames with their time stamps")
wrpcap("c.pcap", p, linktype=227); q = rdpcap("c.pcap")
check(len(q) == 2 and type(q[0]).__name__ == "CAN" and q[0].data == b"\x01\x02\x03" and q[1].identifier == 0x18FECA80, "scapy: the frames cross a pcap file (LINKTYPE_CAN_SOCKETCAN)")
x = UDS() / UDS_RDBI(identifiers=[0xF190])
check(hx(x) == "22f190" and UDS(b"\x62\xf1\x90ABC").haslayer(UDS_RDBIPR), "scapy: UDS ReadDataByIdentifier builds 22 F1 90; 62 F1 90 dissects as its positive response")
fr = ISOTP(b"\x22\xf1\x90" * 5, tx_id=0x7e0).fragment()
check(len(fr) == 3 and hx(fr[0].data) == "100f22f19022f190" and fr[1].data[0] == 0x21, "scapy: a 15-byte ISO-TP message fragments into FF 10 0F and two CFs")

# ---- scapy on a private CAN bus
qd = subprocess.Popen(["qcand", "-f", "-s", os.environ["QCAN_SOCKET"], "vcan0"], stdout=open("qcand.out", "w"), stderr=subprocess.STDOUT)
open("qcand.pid", "w").write(str(qd.pid))
for i in range(50):
    if os.path.exists(os.environ["QCAN_SOCKET"]): break
    time.sleep(0.1)
s1 = CANSocket(bustype="socketcan", channel="vcan0"); s2 = CANSocket(bustype="socketcan", channel="vcan0")
s1.send(f); r = s2.recv()
check(r is not None and r.identifier == 0x123 and r.data == b"\x01\x02\x03", "scapy: CANSocket (python-can, qnx-socketcan) sends and receives 123#010203")
a = ISOTPSoftSocket(s1, tx_id=0x7e0, rx_id=0x7e8, basecls=UDS); b = ISOTPSoftSocket(s2, tx_id=0x7e8, rx_id=0x7e0, basecls=UDS)
got = []
def srv():
    x = b.sniff(count=1, timeout=5)
    if x: got.append(bytes(x[0])); b.send(UDS(b"\x62\xf1\x90" + b"V" * 17))
t = threading.Thread(target=srv); t.start(); time.sleep(0.3)
r = a.sr1(UDS() / UDS_RDBI(identifiers=[0xF190]), timeout=5); t.join()
check(got == [b"\x22\xf1\x90"] and r is not None and r.haslayer(UDS_RDBIPR) and bytes(r) == b"\x62\xf1\x90" + b"V" * 17, "scapy: ISOTPSoftSocket pair: UDS request 22 F1 90, segmented 20-byte response")
a.close(); b.close(); s1.close(); s2.close()
qd.terminate()

# ---- scapy on the network (io-sock: PF_ROUTE, /dev/bpf)
ifs = get_if_list()
check("lo0" in ifs and conf.route.route("127.0.0.1")[0] == "lo0", "scapy: interfaces %r, 127.0.0.1 is routed to lo0" % ifs)
if os.geteuid() != 0 or not os.access("/dev/bpf", os.R_OK | os.W_OK):
    print("  info no access to /dev/bpf (not root): live send/receive not run")
else:
    # Wait until the sniffer is capturing (its first start takes longer than a
    # fixed pause when numpy is installed and pint has loaded it).
    up = threading.Event()
    sn = AsyncSniffer(iface="lo0", filter="icmp", count=2, started_callback=up.set); sn.start(); up.wait(30); time.sleep(0.2)
    r = sr1(IP(dst="127.0.0.1") / ICMP(id=7, seq=9) / b"qnx", timeout=3)
    check(r is not None and r[ICMP].type == 0 and r[ICMP].id == 7 and r[ICMP].seq == 9 and bytes(r[Raw]) == b"qnx", "scapy: sr1 ICMP echo on lo0 through /dev/bpf is answered")
    sn.join(timeout=5)
    if sn.running: sn.stop()
    cap = sn.results or []
    check(len(cap) == 2 and sorted(x[ICMP].type for x in cap) == [0, 8], "scapy: a sniffer with the filter \"icmp\" (libpcap) sees request and reply: %r %r" % ([x.summary() for x in cap], getattr(sn, "exception", None)))
print("FAILS %d" % fails)
P
"$PY" t.py > out.txt 2>&1; rc=$?
cat out.txt
if [ $rc -eq 0 ] && grep -q "^FAILS 0$" out.txt; then echo "PASS python3-canapps"; exit 0; fi
echo "FAIL python3-canapps (rc=$rc)"; exit 1
