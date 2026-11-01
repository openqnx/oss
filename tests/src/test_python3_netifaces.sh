#!/bin/sh
# python3-netifaces: the interface list includes the loopback interface lo0,
# whose AF_INET address is 127.0.0.1 with netmask 255.0.0.0; it has an IPv6
# ::1 as well (when the stack has IPv6); every interface name answers
# ifaddresses(); an unknown name raises ValueError; the address-family
# constants are the platform's socket module values; gateways() returns
# its documented shape (a target with loopback only may have no default).
echo "== python3-netifaces =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-netifaces.XXXXXX") || { echo "FAIL python3-netifaces (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import sys, socket
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
try:
    import netifaces as ni
except Exception as e:
    print("  FAIL import netifaces: %r" % e); sys.exit(1)
info("netifaces %s" % ni.version)
ifs = ni.interfaces()
info("interfaces: %s" % " ".join(ifs))
check(isinstance(ifs, list) and "lo0" in ifs, "interfaces() lists lo0")
check(ni.AF_INET == socket.AF_INET and ni.AF_INET6 == socket.AF_INET6, "AF_INET/AF_INET6 match the socket module")
lo = ni.ifaddresses("lo0") if "lo0" in ifs else {}
v4 = lo.get(ni.AF_INET, [])
info("lo0 AF_INET: %r" % v4)
check(any(a.get("addr") == "127.0.0.1" for a in v4), "lo0 has 127.0.0.1")
check(any(a.get("addr") == "127.0.0.1" and a.get("netmask") == "255.0.0.0" for a in v4), "lo0 127.0.0.1 has netmask 255.0.0.0")
v6 = [a.get("addr", "") for a in lo.get(ni.AF_INET6, [])]
if socket.has_ipv6 and v6:
    check("::1" in v6, "lo0 has ::1")
else:
    info("no IPv6 on lo0")
ok = True
for i in ifs:
    try:
        d = ni.ifaddresses(i)
        ok = ok and isinstance(d, dict)
    except Exception as e:
        info("%s: %r" % (i, e)); ok = False
check(ok, "ifaddresses() answers for every interface")
try:
    ni.ifaddresses("qnx-no-such-if0"); check(False, "unknown interface raises ValueError")
except ValueError:
    check(True, "unknown interface raises ValueError")
g = ni.gateways()
info("gateways: %r" % (g,))
check(isinstance(g, dict) and "default" in g and isinstance(g["default"], dict), "gateways() returns {'default': {...}, family: [...]}")
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-netifaces"; exit 0; }
echo "FAIL python3-netifaces (rc=$rc)"; exit 1
