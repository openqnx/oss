#!/bin/sh
# ntpsec: its Python library over libntpc (loaded with ctypes) -- version,
# status words, NTP timestamps, and the MAC algorithms against known vectors
# (legacy MD5/SHA1 digests, AES-CMAC from RFC 4493 and NTPsec's own vectors);
# ntpkeygen writes a key file the Authenticator loads and signs packets with;
# ntpd reports its version; and ntpdig queries an SNTP server on loopback
# (port 123, run here; needs root) -- unauthenticated and with an AES key --
# and reports the server's 100 s offset.
echo "== ntpsec =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-ntpsec.XXXXXX") || { echo "FAIL ntpsec (no temporary directory)"; exit 1; }
cd "$T" || exit 1
cat > t.py <<'P'
import sys, os
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
def run(name, fn):
    try: fn()
    except Exception as e:
        import traceback; traceback.print_exc(limit=3, file=sys.stdout)
        check(False, "%s: %r" % (name, e))
import subprocess, hashlib, socket, struct, threading, time, json, ctypes
BIN = "/usr/local/bin" if os.path.isdir("/usr/local/bin") else "/data/oss/usr/local/bin"
def lib():
    import ntp.ntpc, ntp.util, ntp.poly
    v = ctypes.c_char_p.in_dll(ntp.ntpc._ntpc, "version").value
    info("libntpc %s" % v.decode())
    check(v == b"1.2.4+NTPsec_1_2_4-dirty" and ntp.util.stdversion() == "ntpsec-1.2.4+NTPsec_1_2_4-dirty", "libntpc loaded through ctypes; version matches the module")
    check(ntp.ntpc.statustoa(ntp.ntpc.TYPE_SYS, 0x12345678) == "leap_add_sec, sync_22, 7 events, no_sys_peer", "statustoa (system status word)")
    check(ntp.ntpc.statustoa(ntp.ntpc.TYPE_PEER, 0x12345678) == "authenb, reach, sel_sys.peer, 7 events, access_denied", "statustoa (peer status word)")
    check(abs(ntp.ntpc.lfptofloat("0xcfba1ce0.80000000") - 1276092000.5) < 1e-6, "lfptofloat 0xcfba1ce0.80000000 = 1276092000.5")
    check(ntp.ntpc.prettydate("0xcfba1ce0.80000000") == "cfba1ce0.80000000 2010-06-09T14:00:00.500Z", "prettydate -> 2010-06-09T14:00:00.500Z")
def macs():
    import ntp.ntpc, ntp.util, ntp.poly
    key, data = b"qnxkey", b"x" * 48
    check(ntp.ntpc.mac(data, key, "MD5") == hashlib.md5(key + data).digest(), "MD5 MAC = md5(key || data)")
    check(ntp.ntpc.mac(data, key, "SHA1") == hashlib.sha1(key + data).digest(), "SHA1 MAC = sha1(key || data)")
    k = bytes.fromhex("2b7e151628aed2a6abf7158809cf4f3c"); m = bytes.fromhex("6bc1bee22e409f96e93d7e117393172a")
    check(ntp.ntpc.mac(m, k, "AES") == bytes.fromhex("070a16b46b4d4144f79bdd9dd04a287c"), "AES-128-CMAC, RFC 4493 example 2")
    key = ntp.util.hexstr2octets("0fd2287c1e97a50cb9d3cb9f80debcb6")
    sample = ntp.util.hexstr2octets("e3000000000000000000000000000000" "00000000000000000000000000000000"
                                    "0000000000000000e6136a712d8c4800" "00000064" "01c167a4df28f25a4794bf00c8b35469")
    # hexstr2octets gives a str of octets; mac() returns bytes
    check(ntp.ntpc.mac(ntp.poly.polybytes(sample[:48]), ntp.poly.polybytes(key), "aes-128") == ntp.poly.polybytes(sample[52:]),
          "AES-128 CMAC of an NTP packet (NTPsec test vector)")
    check(ntp.ntpc.checkname("AES") and not ntp.ntpc.checkname("NOPE"), "checkname: AES known, NOPE not")
def keys():
    import ntp.packet
    p = subprocess.run([sys.executable, BIN + "/ntpkeygen"], capture_output=True, text=True)
    check(p.returncode == 0 and os.path.islink("ntp.keys"), "ntpkeygen wrote a key file and the ntp.keys link")
    a = ntp.packet.Authenticator("ntp.keys")
    check(len(a) == 20 and all(a[i][0] == "AES-128" for i in range(1, 21)), "Authenticator loads 20 AES-128 keys")
    pkt = ntp.packet.SyncPacket(); pkt.transmit_timestamp = ntp.packet.SyncPacket.posix_to_ntp(time.time()); body = pkt.flatten()
    t, pw = a[11]
    signed = body + ntp.packet.Authenticator.compute_mac(body, 11, t, pw)
    check(len(signed) == 68 and a.verify_mac(signed), "packet signed with key 11 verifies (48 + 4 + 16 bytes)")
    check(not a.verify_mac(signed[:-1] + bytes([signed[-1] ^ 1])), "a flipped MAC bit does not")
def daemon():
    p = subprocess.run([BIN.replace("/bin", "/sbin") + "/ntpd", "--version"], capture_output=True, text=True)
    info((p.stdout + p.stderr).strip())
    check(p.returncode == 0 and "ntpsec-1.2.4" in p.stdout + p.stderr, "ntpd --version: ntpsec-1.2.4")
def sntp():
    import ntp.packet
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(("127.0.0.1", 123))
    except OSError as e:
        info("cannot bind 127.0.0.1:123 (%s): ntpdig not exercised" % e); return
    auth = ntp.packet.Authenticator("ntp.keys") if os.path.exists("ntp.keys") else None
    mode = {"mac": None}; stop = []; seen = []
    def serve():
        s.settimeout(0.2)
        while not stop:
            try: d, peer = s.recvfrom(1024)
            except socket.timeout: continue
            t2 = time.time() + 100.0
            req = ntp.packet.SyncPacket(d[:48])
            r = struct.pack("!BBBbIII", (0 << 6) | (4 << 3) | 4, 2, 4, -20, 0, 0, 0x514E5800)
            r += struct.pack("!QQQQ", ntp.packet.SyncPacket.posix_to_ntp(t2 - 10), req.transmit_timestamp,
                             ntp.packet.SyncPacket.posix_to_ntp(t2), ntp.packet.SyncPacket.posix_to_ntp(time.time() + 100.0))
            if len(d) > 48 and auth:
                keyid = struct.unpack("!I", d[48:52])[0]; t, pw = auth[keyid]
                seen.append((keyid, auth.verify_mac(d, packet_end=48, mac_begin=48)))
                mac = ntp.packet.Authenticator.compute_mac(r, keyid, t, pw)
                if mode["mac"] == "bad": mac = mac[:-1] + bytes([mac[-1] ^ 0xFF])
                r += mac
            s.sendto(r, peer)
    th = threading.Thread(target=serve); th.start()
    def dig(*extra):
        p = subprocess.run([sys.executable, BIN + "/ntpdig", "-j", "-t", "3"] + list(extra) + ["127.0.0.1"], capture_output=True, text=True, timeout=30)
        line = [l for l in p.stdout.splitlines() if l.startswith("{")]
        if not line: info("ntpdig %s: rc %d %s" % (" ".join(extra), p.returncode, (p.stdout + p.stderr).strip()[-800:]))
        return json.loads(line[0]) if line else None
    try:
        j = dig()
        if j: info("ntpdig: offset %.3f s, stratum %s, host %s" % (j["offset"], j["stratum"], j["ip"]))
        check(j is not None and abs(j["offset"] - 100.0) < 1.0 and j["stratum"] == 2, "ntpdig against the loopback server: offset +100 s, stratum 2")
        if auth:
            j = dig("-a", "11", "-k", "ntp.keys")
            info("server saw (keyid, request MAC verifies): %s" % seen)
            check(seen and all(v for _, v in seen), "the server verifies ntpdig's AES-CMAC request MAC (key 11)")
            check(j is not None and abs(j["offset"] - 100.0) < 1.0, "ntpdig -a 11 -k ntp.keys: AES-CMAC authenticated reply accepted")
            mode["mac"] = "bad"
            check(dig("-a", "11", "-k", "ntp.keys") is None, "a reply with a wrong MAC is refused")
    finally:
        stop.append(1); th.join(); s.close()
for n, f in (("libntpc", lib), ("mac", macs), ("keys", keys), ("ntpd", daemon), ("ntpdig", sntp)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS ntpsec"; exit 0; }
echo "FAIL ntpsec (rc=$rc)"; exit 1
