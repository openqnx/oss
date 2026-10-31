#!/bin/sh
# python3-wheel: the wheel tool on the ported CPython -- write a wheel with
# WheelFile (RECORD hashes generated), read and verify it, unpack it, modify
# and repack it (RECORD regenerated), retag it, and reject a wheel whose
# contents no longer match its RECORD.
echo "== python3-wheel =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-wheel.XXXXXX") || { echo "FAIL python3-wheel (no temporary directory)"; exit 1; }
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
import subprocess, zipfile, hashlib, base64, shutil
import wheel
from wheel.wheelfile import WheelFile
info("wheel %s" % wheel.__version__)
W = "qnxdemo-0.1-py3-none-any.whl"
def cli(*a):
    p = subprocess.run([sys.executable, "-m", "wheel"] + list(a), capture_output=True, text=True)
    if p.returncode: info("wheel %s: rc %d: %s" % (" ".join(a), p.returncode, (p.stderr or p.stdout).strip()[-300:]))
    return p
def write():
    with WheelFile(W, "w") as wf:
        wf.writestr("qnxdemo/__init__.py", "ANSWER = 42\n")
        wf.writestr("qnxdemo-0.1.dist-info/METADATA", "Metadata-Version: 2.1\nName: qnxdemo\nVersion: 0.1\n")
        wf.writestr("qnxdemo-0.1.dist-info/WHEEL", "Wheel-Version: 1.0\nGenerator: qnx\nRoot-Is-Purelib: true\nTag: py3-none-any\n")
    z = zipfile.ZipFile(W); rec = z.read("qnxdemo-0.1.dist-info/RECORD").decode()
    h = base64.urlsafe_b64encode(hashlib.sha256(b"ANSWER = 42\n").digest()).rstrip(b"=").decode()
    check("qnxdemo/__init__.py,sha256=%s,12" % h in rec, "WheelFile wrote RECORD with the sha256 of each file")
    with WheelFile(W) as wf:
        check(wf.read("qnxdemo/__init__.py") == b"ANSWER = 42\n" and wf.parsed_filename.group("name") == "qnxdemo", "WheelFile reads it back, hashes verified")
def unpack_pack():
    p = cli("unpack", "-d", "out", W)
    check(p.returncode == 0 and os.path.isfile("out/qnxdemo-0.1/qnxdemo/__init__.py"), "wheel unpack")
    open("out/qnxdemo-0.1/qnxdemo/extra.py", "w").write("X = 1\n")
    os.makedirs("re"); p = cli("pack", "-d", "re", "out/qnxdemo-0.1")
    ok = p.returncode == 0 and os.path.isfile("re/" + W)
    check(ok, "wheel pack after adding a file")
    if ok:
        rec = zipfile.ZipFile("re/" + W).read("qnxdemo-0.1.dist-info/RECORD").decode()
        check("qnxdemo/extra.py,sha256=" in rec, "RECORD regenerated with the new file")
    p = cli("tags", "--python-tag", "py311.py312", "--remove", "re/" + W)
    tagged = [f for f in os.listdir("re") if f.endswith(".whl")]
    info("after tags: %s" % tagged)
    check(p.returncode == 0 and tagged == ["qnxdemo-0.1-py311.py312-none-any.whl"], "wheel tags --python-tag py311.py312")
def tampered():
    os.makedirs("bad"); B = "bad/" + W
    src = zipfile.ZipFile(W)
    with zipfile.ZipFile(B, "w") as z:              # same RECORD, one file changed
        for n in src.namelist():
            z.writestr(n, b"ANSWER = 666\n" if n == "qnxdemo/__init__.py" else src.read(n))
    bad = False
    try:
        with WheelFile(B) as wf:
            for n in wf.namelist(): wf.read(n)
    except Exception as e: bad = True; info("rejected: %s" % e)
    check(bad, "a file whose hash no longer matches RECORD is rejected")
for n, f in (("write", write), ("unpack/pack", unpack_pack), ("tampered", tampered)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-wheel"; exit 0; }
echo "FAIL python3-wheel (rc=$rc)"; exit 1
