#!/bin/sh
# The Python CAN libraries on top of python3-can, each with known answers:
#  - bitstruct: pack/unpack of bit fields, the same bytes from the pure
#    Python module and the C extension (bitstruct.c), byte swapping;
#  - crccheck: the check values of CRC-32, CRC-16/CCITT-FALSE, CRC-8 and
#    CRC-16/MODBUS over "123456789", an 8-bit checksum;
#  - argparse_addons: Integer(min, max) parses 0x10 and refuses 256;
#  - textparser: a token grammar parses "Hello, World!" into its tokens and
#    reports the position of a syntax error;
#  - cantools: a DBC database (little and big endian, signed, scaled,
#    enumerated and multiplexed signals) encodes to the expected bytes and
#    decodes back; a value out of range is refused; the cantools command
#    decodes candump lines; a KCD dump of the database loads again;
#  - canmatrix: the same DBC loads (frames, signals, factor/offset, value
#    table), encodes and decodes a frame, converts to JSON and SYM
#    (canconvert) and back with the same signals; cancompare reports a
#    changed signal;
#  - can-isotp (isotp): two transport stacks on a private qcand bus
#    exchange a 7-byte single frame and a 300-byte segmented message (first
#    frame "11 2C", flow control "30", consecutive frames on the wire), with
#    CAN FD frames, and with extended (29-bit) addressing; a sender without
#    a receiver times out waiting for flow control;
#  - udsoncan: a UDS client talks through isotp to a small server on the
#    bus: session control, ReadDataByIdentifier (a 17-character VIN, a
#    segmented response), WriteDataByIdentifier, ECU reset, tester present;
#    an unsupported identifier raises NegativeResponseException with
#    requestOutOfRange;
#  - can-j1939 (j1939): an ECU sends PGN 65226 with source address 0x80 and
#    another receives priority, PGN, source and data (CAN id 18FECA80 on
#    the wire); a 20-byte message crosses as a BAM transport session
#    (TP.CM/TP.DT frames) and is reassembled; a controller application
#    claims its address;
#  - pyserial (serial): the loop:// port echoes; a pseudo terminal pair
#    carries bytes both ways at 115200 and at a rate termios has no name
#    for (250000, read back from the terminal), in_waiting counts, RTS/CTS
#    flow control sets QNX's IHFLOW|OHFLOW, read_until; list_ports finds
#    the /dev/ser* devices devc-ser* registered.
# The Linux kernel transports are not there on QNX: isotp.socket (CAN_ISOTP)
# is not used, the transport layers above run in Python over CAN_RAW.
echo "== python3-canlibs =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pycanlibs.XXXXXX") || { echo "FAIL python3-canlibs (no temporary directory)"; exit 1; }
trap 'cd /; [ -f "$T/qcand.pid" ] && kill "$(cat "$T/qcand.pid")" 2>/dev/null; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1
QCAN_SOCKET="$T/qcand.sock"; export QCAN_SOCKET
cat > t.dbc <<'D'
VERSION "1.0"

NS_ :

BS_:

BU_: ECU1 ECU2

BO_ 256 EngineData: 8 ECU1
 SG_ EngineSpeed : 0|16@1+ (0.25,0) [0|16383.75] "rpm" ECU2
 SG_ CoolantTemp : 16|8@1+ (1,-40) [-40|215] "degC" ECU2
 SG_ Torque : 24|12@1- (0.5,0) [-1024|1023.5] "Nm" ECU2
 SG_ Gear : 36|4@1+ (1,0) [0|15] "" ECU2
 SG_ Counter : 47|16@0+ (1,0) [0|65535] "" ECU2

BO_ 512 MuxMsg: 4 ECU2
 SG_ Mode M : 0|8@1+ (1,0) [0|255] "" ECU1
 SG_ ValueA m0 : 8|16@1+ (1,0) [0|65535] "" ECU1
 SG_ ValueB m1 : 8|8@1- (1,0) [-128|127] "" ECU1

BO_ 2566844158 ExtMsg: 2 ECU1
 SG_ Level : 0|16@1+ (0.1,0) [0|6553.5] "l" ECU2

CM_ SG_ 256 EngineSpeed "Engine speed";
VAL_ 256 Gear 0 "Neutral" 1 "First" 2 "Second" 15 "Reverse" ;
D
cat > t.py <<'P'
import sys, os, time, subprocess, threading, json
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok); sys.stdout.flush()
def hx(b): return bytes(b).hex()

# ---- bitstruct
import bitstruct, bitstruct.c as bc
check(bitstruct.pack('u1u3u4s16', 1, 2, 3, -4) == b'\xa3\xff\xfc' and bc.pack('u1u3u4s16', 1, 2, 3, -4) == b'\xa3\xff\xfc', "bitstruct: pack('u1u3u4s16', 1, 2, 3, -4) is a3fffc (Python and C)")
check(bc.unpack('u1u3u4s16', b'\xa3\xff\xfc') == (1, 2, 3, -4) and bitstruct.unpack('u1u3u4s16', b'\xa3\xff\xfc') == (1, 2, 3, -4), "bitstruct: unpack gives (1, 2, 3, -4)")
check(bitstruct.pack('f32t16r8', 1.5, 'ab', b'\x7f') == b'\x3f\xc0\x00\x00ab\x7f' and bitstruct.byteswap('12', b'\x00\x11\x22') == b'\x00\x22\x11' and bitstruct.calcsize('u1u3u4s16') == 24, "bitstruct: float, text and raw fields; byteswap; calcsize")
cf = bc.compile('u4u4'); check(cf.pack(0xa, 0x5) == b'\xa5' and cf.unpack(b'\x3c') == (3, 12), "bitstruct.c: a compiled format packs and unpacks")

# ---- crccheck
from crccheck.crc import Crc32, Crc16CcittFalse, Crc8, Crc16Modbus
from crccheck.checksum import Checksum8
d = b"123456789"
check(Crc32.calc(d) == 0xCBF43926 and Crc16CcittFalse.calc(d) == 0x29B1 and Crc8.calc(d) == 0xF4 and Crc16Modbus.calc(d) == 0x4B37, "crccheck: CRC-32 cbf43926, CRC-16/CCITT-FALSE 29b1, CRC-8 f4, CRC-16/MODBUS 4b37")
c = Crc32(); c.process(b"1234"); c.process(b"56789")
check(c.final() == 0xCBF43926 and Checksum8.calc(d) == 221 and Crc32.calcbytes(d) == bytes.fromhex("cbf43926"), "crccheck: incremental CRC, calcbytes, 8-bit checksum 221")

# ---- argparse_addons
import argparse, argparse_addons
check(argparse_addons.Integer(0, 255)('0x10') == 16, "argparse_addons: Integer(0, 255) parses 0x10")
try: argparse_addons.Integer(0, 255)('256'); check(False, "argparse_addons: 256 is refused")
except argparse.ArgumentTypeError as e: check("0..255" in str(e), "argparse_addons: 256 is refused (%s)" % e)

# ---- textparser
import textparser
from textparser import Sequence, ZeroOrMore, choice
class Parser(textparser.Parser):
    def token_specs(self):
        return [('SKIP', r'[ \r\n\t]+'), ('WORD', r'\w+'), ('EMARK', '!', r'!'), ('COMMA', ',', r','), ('MISMATCH', r'.')]
    def grammar(self):
        return Sequence('WORD', ZeroOrMore(Sequence(',', 'WORD')), '!')
tree = Parser().parse('Hello, World!')
check(tree == ['Hello', [[',', 'World']], '!'], "textparser: 'Hello, World!' parses into its tokens: %r" % (tree,))
try: Parser().parse('Hello, !'); check(False, "textparser: a syntax error is reported")
except textparser.ParseError as e: check(e.line == 1 and e.column == 8, "textparser: a syntax error is reported at line 1, column 8 (%s)" % str(e).split("\n")[0])

# ---- cantools
import cantools
db = cantools.database.load_file("t.dbc")
em = db.get_message_by_name("EngineData")
check(sorted(m.name for m in db.messages) == ["EngineData", "ExtMsg", "MuxMsg"] and em.frame_id == 0x100 and em.length == 8 and db.get_message_by_name("ExtMsg").is_extended_frame and db.get_message_by_name("ExtMsg").frame_id == 0x18FEEEFE, "cantools: the DBC loads: three messages, ExtMsg extended 18FEEEFE")
sig = {"EngineSpeed": 3000.0, "CoolantTemp": 90, "Torque": -100.5, "Gear": "Second", "Counter": 0x1234}
enc = em.encode(sig)
check(hx(enc) == "e02e82372f123400", "cantools: EngineData encodes to e02e82372f123400: " + hx(enc))
dec = em.decode(bytes.fromhex("e02e82372f123400"))
check(dec["EngineSpeed"] == 3000.0 and dec["CoolantTemp"] == 90 and dec["Torque"] == -100.5 and str(dec["Gear"]) == "Second" and dec["Counter"] == 0x1234, "cantools: and decodes back (3000 rpm, 90 degC, -100.5 Nm, Second, 0x1234)")
raw = em.decode(bytes.fromhex("e02e82372f123400"), decode_choices=False, scaling=False)
check(raw["EngineSpeed"] == 12000 and raw["CoolantTemp"] == 130 and raw["Torque"] == -201 and raw["Gear"] == 2, "cantools: raw values 12000, 130, -201, 2")
mm = db.get_message_by_name("MuxMsg")
check(hx(mm.encode({"Mode": 0, "ValueA": 0xBEEF})) == "00efbe00" and hx(mm.encode({"Mode": 1, "ValueB": -2})) == "01fe0000" and mm.decode(bytes.fromhex("01fe0000")) == {"Mode": 1, "ValueB": -2}, "cantools: multiplexed signals: mode 0 -> 00efbe00, mode 1 -> 01fe0000")
try: em.encode(dict(sig, CoolantTemp=300)); check(False, "cantools: a value out of range is refused")
except cantools.database.EncodeError as e: check(True, "cantools: a value out of range is refused")
check(abs(db.get_message_by_name("ExtMsg").decode(b"\x39\x30")["Level"] - 1234.5) < 1e-9, "cantools: ExtMsg 3930 is 1234.5 l")
p = subprocess.run([sys.executable, "-m", "cantools", "decode", "--single-line", "t.dbc"], input="(1700000000.000000) vcan0 100#E02E82372F123400\n", capture_output=True, text=True)
check(p.returncode == 0 and "EngineData(EngineSpeed: 3000.0 rpm" in p.stdout and "Gear: Second" in p.stdout, "cantools decode prints the decoded candump line: " + p.stdout.strip()[-110:] + p.stderr[-200:])
cantools.database.dump_file(db, "t.kcd"); db2 = cantools.database.load_file("t.kcd")
check(hx(db2.get_message_by_name("EngineData").encode(sig)) == "e02e82372f123400", "cantools: a KCD dump loads again and encodes the same bytes")

# ---- canmatrix
import canmatrix, canmatrix.formats
cm = canmatrix.formats.loadp_flat("t.dbc")
fr = cm.frame_by_name("EngineData")
s0 = fr.signal_by_name("EngineSpeed"); g = fr.signal_by_name("Gear")
check(len(cm.frames) == 3 and fr.arbitration_id.id == 0x100 and fr.size == 8 and len(fr.signals) == 5 and float(s0.factor) == 0.25 and s0.size == 16 and s0.unit == "rpm" and g.values[15] == "Reverse" and cm.frame_by_name("ExtMsg").arbitration_id.extended, "canmatrix: the DBC loads: 3 frames, EngineSpeed 16 bits factor 0.25 rpm, Gear 15 = Reverse")
enc = fr.encode({"EngineSpeed": 12000, "CoolantTemp": 130, "Torque": -201, "Gear": 2, "Counter": 0x1234})
check(hx(enc) == "e02e82372f123400", "canmatrix: EngineData encodes (raw values) to e02e82372f123400: " + hx(enc))
dec = fr.decode(bytes.fromhex("e02e82372f123400"))
check(float(dec["EngineSpeed"].phys_value) == 3000.0 and dec["Torque"].raw_value == -201 and dec["Gear"].named_value == "Second", "canmatrix: and decodes (3000.0, raw torque -201, Second)")
cc = os.path.join(os.path.dirname(sys.executable), "canconvert"); cp = os.path.join(os.path.dirname(sys.executable), "cancompare")
p = subprocess.run([cc, "t.dbc", "t.json"], capture_output=True, text=True)
j = json.load(open("t.json")) if os.path.exists("t.json") else {}
check(p.returncode == 0 and sorted(m["name"] for m in j.get("messages", [])) == ["EngineData", "ExtMsg", "MuxMsg"], "canconvert DBC -> JSON keeps the three messages " + p.stderr[-200:])
p = subprocess.run([cc, "t.dbc", "t.sym"], capture_output=True, text=True)
back = canmatrix.formats.loadp_flat("t.sym") if os.path.exists("t.sym") else None
bs = back.frame_by_name("EngineData").signal_by_name("Torque") if back else None
check(p.returncode == 0 and bs is not None and bs.is_signed and bs.size == 12 and float(bs.factor) == 0.5, "canconvert DBC -> SYM and back keeps Torque (12 bits, signed, factor 0.5) " + p.stderr[-200:])
open("t2.dbc", "w").write(open("t.dbc").read().replace("16|8@1+ (1,-40)", "16|8@1+ (1,-50)"))
p = subprocess.run([cp, "t.dbc", "t2.dbc"], capture_output=True, text=True)
check("CoolantTemp" in p.stdout + p.stderr and "EngineSpeed" not in p.stdout + p.stderr, "cancompare reports the changed CoolantTemp only")

# ---- a private bus
import can
qd = subprocess.Popen(["qcand", "-f", "-s", os.environ["QCAN_SOCKET"], "vcan0", "vcan1:classic"], stdout=open("qcand.out", "w"), stderr=subprocess.STDOUT)
open("qcand.pid", "w").write(str(qd.pid))
for i in range(50):
    if os.path.exists(os.environ["QCAN_SOCKET"]): break
    time.sleep(0.1)
def drain(bus, t=0.3):
    out = []
    while True:
        r = bus.recv(t)
        if r is None: return out
        out.append(r)
try:
    # ---- isotp
    import isotp
    def pair(chan, fd, a1, a2, **params):
        b1 = can.Bus(chan, interface="socketcan", fd=fd); b2 = can.Bus(chan, interface="socketcan", fd=fd)
        s1 = isotp.CanStack(b1, address=a1, params=params); s2 = isotp.CanStack(b2, address=a2, params=params)
        s1.start(); s2.start(); return b1, b2, s1, s2
    def done(b1, b2, s1, s2):
        s1.stop(); s2.stop(); b1.shutdown(); b2.shutdown()
    N11 = isotp.AddressingMode.Normal_11bits
    mon = can.Bus("vcan1", interface="socketcan")
    x = pair("vcan1", False, isotp.Address(N11, txid=0x7E0, rxid=0x7E8), isotp.Address(N11, txid=0x7E8, rxid=0x7E0), stmin=1, blocksize=8)
    b1, b2, s1, s2 = x
    s1.send(b"\x01\x02\x03\x04\x05\x06\x07"); r = s2.recv(block=True, timeout=2)
    fr = drain(mon)
    check(r == b"\x01\x02\x03\x04\x05\x06\x07" and len(fr) == 1 and hx(fr[0].data) == "0701020304050607" and fr[0].arbitration_id == 0x7E0, "isotp: 7 bytes cross as one single frame 7E0#0701020304050607")
    big = bytes((i * 7 + 3) & 0xff for i in range(300))
    s1.send(big); r = s2.recv(block=True, timeout=5); fr = drain(mon)
    ff = [f for f in fr if f.arbitration_id == 0x7E0]; fc = [f for f in fr if f.arbitration_id == 0x7E8]
    check(r == big, "isotp: a 300-byte message is reassembled")
    check(len(ff) == 43 and hx(ff[0].data[:2]) == "112c" and bytes(ff[0].data[2:]) == big[:6] and ff[1].data[0] == 0x21 and ff[16].data[0] == 0x20 and len(fc) == 6 and hx(fc[0].data[:3]) == "300801", "isotp: on the wire: first frame 11 2C, 42 consecutive frames (21, 22, ...), 6 flow controls 30 08 01 (%d/%d)" % (len(ff), len(fc)))
    s2.send(big[:100]); r = s1.recv(block=True, timeout=5)
    check(r == big[:100], "isotp: and 100 bytes the other way"); drain(mon)
    done(*x); mon.shutdown()
    x = pair("vcan0", True, isotp.Address(N11, txid=0x700, rxid=0x701), isotp.Address(N11, txid=0x701, rxid=0x700), tx_data_length=64, can_fd=True, stmin=0, blocksize=0)
    mon = can.Bus("vcan0", interface="socketcan", fd=True)
    x[2].send(big); r = x[3].recv(block=True, timeout=5); fr = [f for f in drain(mon) if f.arbitration_id == 0x700]
    check(r == big and len(fr) == 5 and fr[0].is_fd and len(fr[0].data) == 64, "isotp: with CAN FD the 300 bytes take 5 frames of up to 64 bytes (%d)" % len(fr))
    done(*x)
    N29 = isotp.AddressingMode.NormalFixed_29bits
    x = pair("vcan0", False, isotp.Address(N29, source_address=0xF1, target_address=0x10), isotp.Address(N29, source_address=0x10, target_address=0xF1), stmin=0, blocksize=0)
    x[2].send(big[:50]); r = x[3].recv(block=True, timeout=5); fr = drain(mon)
    check(r == big[:50] and fr and fr[0].is_extended_id and fr[0].arbitration_id == 0x18DA10F1, "isotp: 29-bit fixed addressing uses id 18DA10F1")
    done(*x)
    errs = []
    b1 = can.Bus("vcan0", interface="socketcan")
    s1 = isotp.CanStack(b1, address=isotp.Address(N11, txid=0x7A0, rxid=0x7A8), params={"rx_flowcontrol_timeout": 300}, error_handler=errs.append)
    s1.start(); s1.send(big[:20]); time.sleep(0.8); s1.stop(); b1.shutdown(); drain(mon); mon.shutdown()
    check(any(isinstance(e, isotp.FlowControlTimeoutError) for e in errs), "isotp: without a receiver the sender reports FlowControlTimeoutError")

    # ---- udsoncan
    import udsoncan
    from udsoncan.connections import PythonIsoTpConnection
    from udsoncan.client import Client
    from udsoncan import services
    VIN = b"WQNX0SOCKETCAN017"; store = {0x0102: b"\x00\x2a"}; seen = []
    sb = can.Bus("vcan0", interface="socketcan")
    srv = isotp.CanStack(sb, address=isotp.Address(N11, txid=0x7E8, rxid=0x7E0), params={"stmin": 0, "blocksize": 0})
    srv.start(); stop = threading.Event()
    def server():
        while not stop.is_set():
            q = srv.recv(block=True, timeout=0.1)
            if q is None: continue
            seen.append(q.hex()); sid = q[0]
            if sid == 0x10: a = bytes([0x50, q[1], 0x07, 0xd0, 0x01, 0xf4])
            elif sid == 0x3E: a = bytes([0x7E, q[1]])
            elif sid == 0x11: a = bytes([0x51, q[1]])
            elif sid == 0x22:
                did = (q[1] << 8) | q[2]
                if did == 0xF190: a = b"\x62\xf1\x90" + VIN
                elif did in store: a = b"\x62" + q[1:3] + store[did]
                else: a = bytes([0x7F, 0x22, 0x31])
            elif sid == 0x2E:
                store[(q[1] << 8) | q[2]] = bytes(q[3:]); a = b"\x6e" + q[1:3]
            else: a = bytes([0x7F, sid, 0x11])
            srv.send(a)
    th = threading.Thread(target=server, daemon=True); th.start()
    cb = can.Bus("vcan0", interface="socketcan")
    cst = isotp.CanStack(cb, address=isotp.Address(N11, txid=0x7E0, rxid=0x7E8), params={"stmin": 0, "blocksize": 0})
    cfg = dict(udsoncan.configs.default_client_config)
    cfg["data_identifiers"] = {0xF190: udsoncan.AsciiCodec(17), 0x0102: ">H", 0x0999: ">H"}
    cfg["request_timeout"] = 3; cfg["p2_timeout"] = 2
    with Client(PythonIsoTpConnection(cst), config=cfg) as cl:
        r = cl.change_session(services.DiagnosticSessionControl.Session.extendedDiagnosticSession)
        check(r.positive and r.service_data.session_echo == 3 and r.service_data.p2_server_max == 2.0, "udsoncan: DiagnosticSessionControl(extended) answers 50 03, P2 2 s")
        r = cl.read_data_by_identifier(0xF190)
        check(r.service_data.values[0xF190] == VIN.decode(), "udsoncan: ReadDataByIdentifier F190 returns the 17-character VIN (segmented): " + str(r.service_data.values[0xF190]))
        check(cl.read_data_by_identifier(0x0102).service_data.values[0x0102] == (42,), "udsoncan: identifier 0102 is 42")
        cl.write_data_by_identifier(0x0102, (0xBEEF,))
        check(store[0x0102] == b"\xbe\xef" and cl.read_data_by_identifier(0x0102).service_data.values[0x0102] == (0xBEEF,), "udsoncan: WriteDataByIdentifier stores BEEF, read back")
        r1 = cl.ecu_reset(services.ECUReset.ResetType.hardReset); r2 = cl.tester_present()
        check(r1.positive and r2.positive and "1101" in seen and "3e00" in seen, "udsoncan: ECUReset(hard) and TesterPresent are 11 01 and 3E 00 on the wire")
        try:
            cl.read_data_by_identifier(0x0999); check(False, "udsoncan: an unsupported identifier raises")
        except udsoncan.exceptions.NegativeResponseException as e:
            check(e.response.code == 0x31 and e.response.code_name == "RequestOutOfRange", "udsoncan: an unsupported identifier raises NegativeResponseException (%s)" % e.response.code_name)
    stop.set(); th.join(); srv.stop(); sb.shutdown(); cb.shutdown()

    # ---- j1939
    import j1939
    mon = can.Bus("vcan0", interface="socketcan")
    e1 = j1939.ElectronicControlUnit(); e2 = j1939.ElectronicControlUnit()
    e1.connect(interface="socketcan", channel="vcan0"); e2.connect(interface="socketcan", channel="vcan0")
    got = []; ev = threading.Event()
    def on(priority, pgn, sa, timestamp, data):
        got.append((priority, pgn, sa, bytes(data))); ev.set()
    e2.subscribe(on)
    e1.send_pgn(0, 0xFE, 0xCA, 6, 0x80, [1, 2, 3, 4, 5, 6, 7, 8]); ev.wait(2)
    fr = drain(mon)
    check(got[:1] == [(6, 65226, 0x80, bytes(range(1, 9)))], "j1939: PGN 65226 from source 80 arrives with priority 6 and its 8 bytes: %r" % got[:1])
    check(len(fr) == 1 and fr[0].is_extended_id and fr[0].arbitration_id == 0x18FECA80 and hx(fr[0].data) == "0102030405060708", "j1939: on the wire it is 18FECA80#0102030405060708")
    got.clear(); ev.clear(); long = bytes(range(100, 120))
    e1.send_pgn(0, 0xFE, 0xE3, 6, 0x80, list(long)); ev.wait(5); fr = drain(mon)
    cmf = [f for f in fr if (f.arbitration_id >> 8) & 0xffff == 0xECFF]; dtf = [f for f in fr if (f.arbitration_id >> 8) & 0xffff == 0xEBFF]
    check([g[1:] for g in got[:1]] == [(65251, 0x80, long)], "j1939: a 20-byte PGN 65251 is reassembled from a BAM session")
    check(len(cmf) == 1 and hx(cmf[0].data) == "20140003ffe3fe00" and len(dtf) == 3 and dtf[0].data[0] == 1 and bytes(dtf[0].data[1:]) == long[:7] and bytes(dtf[2].data) == bytes([3]) + long[14:] + b"\xff", "j1939: TP.CM BAM 20 14 00 03 FF E3 FE 00 and three TP.DT frames on the wire")
    name = j1939.Name(arbitrary_address_capable=0, industry_group=j1939.Name.IndustryGroup.Industrial, vehicle_system_instance=1, vehicle_system=1, function=1, function_instance=1, ecu_instance=1, manufacturer_code=666, identity_number=1234567)
    ca = j1939.ControllerApplication(name, 0x90); e1.add_ca(controller_application=ca); ca.start()
    t0 = time.time()
    while ca.state != j1939.ControllerApplication.State.NORMAL and time.time() - t0 < 3: time.sleep(0.05)
    fr = drain(mon)
    ac = [f for f in fr if f.arbitration_id == 0x18EEFF90]
    check(ca.state == j1939.ControllerApplication.State.NORMAL and len(ac) == 1 and int.from_bytes(bytes(ac[0].data), "little") == name.value, "j1939: a controller application claims address 90 (18EEFF90 with its NAME)")
    ca.stop(); e1.disconnect(); e2.disconnect(); mon.shutdown()
finally:
    qd.terminate()

# ---- pyserial
import serial, serial.tools.list_ports, termios, glob
s = serial.serial_for_url("loop://", timeout=1); s.write(b"hello"); r = s.read(5); s.close()
check(r == b"hello" and serial.__version__ == "3.5", "pyserial: the loop:// port echoes")
m = None
for n in range(4, 16):
    try: m = os.open("/dev/ptyp%x" % n, os.O_RDWR); tty = "/dev/ttyp%x" % n; break
    except OSError: pass
if m is None:
    check(False, "pyserial: no free pseudo terminal")
else:
    p = serial.Serial(tty, 115200, timeout=1)
    os.write(m, b"abc\n"); r1 = p.read(4); p.write(b"xyz"); p.flush(); time.sleep(0.1); r2 = os.read(m, 3)
    check(r1 == b"abc\n" and r2 == b"xyz" and termios.tcgetattr(p.fd)[4] == 115200, "pyserial: Serial(%s, 115200) carries bytes both ways" % tty)
    p.baudrate = 250000
    check(termios.tcgetattr(p.fd)[4:6] == [250000, 250000], "pyserial: 250000 baud (no termios name) is set on the terminal")
    p.rtscts = True; hw = termios.tcgetattr(p.fd)[2] & 3; p.rtscts = False
    check(hw == 3 and termios.tcgetattr(p.fd)[2] & 3 == 0, "pyserial: rtscts sets and clears IHFLOW|OHFLOW")
    os.write(m, b"12345"); time.sleep(0.1); n = p.in_waiting
    check(n == 5 and p.read_until(b"4") == b"1234" and p.read(10) == b"5", "pyserial: in_waiting is 5; read_until; a short read returns after the timeout")
    p.close(); os.close(m)
ports = [x.device for x in serial.tools.list_ports.comports()]
check(ports == sorted(glob.glob("/dev/ser[0-9]*") + glob.glob("/dev/serusb[0-9]*")), "pyserial: list_ports finds the /dev/ser* devices: %r" % ports)
print("FAILS %d" % fails)
P
"$PY" t.py > out.txt 2>&1; rc=$?
cat out.txt
if [ $rc -eq 0 ] && grep -q "^FAILS 0$" out.txt; then echo "PASS python3-canlibs"; exit 0; fi
echo "FAIL python3-canlibs (rc=$rc)"; exit 1
