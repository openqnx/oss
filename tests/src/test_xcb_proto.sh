#!/bin/sh
# xcb-proto: the X protocol in XML, and xcbgen, the Python package libxcb's
# code generator is built on. Drive xcbgen as c_client.py does (a module
# with output callbacks) over xproto.xml and randr.xml -- which imports
# other XML files -- and check what it resolves: request opcodes, events,
# errors, enums, reply fields; every XML file parses.
echo "== xcb-proto =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-xcbproto.XXXXXX") || { echo "FAIL xcb-proto (no temporary directory)"; exit 1; }
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
import glob, xml.etree.ElementTree as ET
XCB = "/usr/local/share/xcb" if os.path.isdir("/usr/local/share/xcb") else os.environ.get("QNX_PORT_XCB", "/data/oss/usr/local/share/xcb")
seen = {"request": {}, "event": {}, "error": {}, "enum": {}, "struct": 0}
def rec(kind):
    def f(self, name):
        if kind == "struct": seen["struct"] += 1
        elif kind == "request": seen["request"][name[-1]] = self
        elif kind in ("event", "error"): seen[kind][name[-1]] = self
        elif kind == "enum": seen["enum"][name[-1]] = self
    return f
noop = lambda *a: None
output = {"open": noop, "close": noop, "simple": noop, "enum": rec("enum"), "struct": rec("struct"), "union": noop,
          "request": rec("request"), "eventstruct": noop, "event": rec("event"), "error": rec("error")}
import __main__; __main__.output = output
from xcbgen.state import Module
def generate(f):
    for k in ("request", "event", "error", "enum"): seen[k] = {}
    m = Module(os.path.join(XCB, f), output); m.register(); m.resolve(); m.generate()
    return m
def xmlfiles():
    fs = sorted(glob.glob(os.path.join(XCB, "*.xml")))
    bad = [f for f in fs if ET.parse(f).getroot().tag != "xcb"]
    check(len(fs) >= 25 and not bad, "%d protocol XML files parse, each an <xcb> description" % len(fs))
def xproto():
    m = generate("xproto.xml")
    r = seen["request"]
    info("xproto: %d requests, %d events, %d errors, %d enums" % (len(r), len(seen["event"]), len(seen["error"]), len(seen["enum"])))
    check(len(r) >= 119 and r["CreateWindow"].opcode == "1" and r["MapWindow"].opcode == "8" and r["InternAtom"].opcode == "16" and r["NoOperation"].opcode == "127",
          "requests: CreateWindow 1, MapWindow 8, InternAtom 16, NoOperation 127")
    check(r["InternAtom"].reply is not None and [f.field_name for f in r["InternAtom"].reply.fields if f.visible] == ["atom"], "InternAtom reply: atom")
    ev = seen["event"]
    check("Expose" in ev and ev["Expose"].opcodes.get(("xcb", "Expose")) == "12" and "KeyPress" in ev, "events: Expose opcode 12, KeyPress")
    check({"Window", "Match", "Value"} <= set(seen["error"]), "errors: Window, Match, Value")
    em = seen["enum"].get("EventMask")
    bits = dict((k, int(v)) for k, v in em.bits) if em else {}
    check(em is not None and bits.get("ButtonPress") == 2 and bits.get("Exposure") == 15, "enum EventMask: ButtonPress bit 2, Exposure bit 15")
def randr():
    m = generate("randr.xml")
    r = seen["request"]
    check(m.namespace.is_ext and m.namespace.ext_xname == "RANDR", "randr.xml: extension RANDR")
    check(r["QueryVersion"].opcode == "0" and r["GetScreenResources"].opcode == "8" and "GetCrtcInfo" in r, "QueryVersion 0, GetScreenResources 8, GetCrtcInfo")
    check(m.has_import("xproto") and m.has_import("render"), "imports xproto and render resolved")
for n, f in (("xml", xmlfiles), ("xproto", xproto), ("randr", randr)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS xcb-proto"; exit 0; }
echo "FAIL xcb-proto (rc=$rc)"; exit 1
