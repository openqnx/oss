#!/bin/sh
# python3-pip: pip runs on the ported CPython -- version and the installed
# distributions (pip list / show, JSON), file hashing, and an offline
# install: a wheel built here is installed into a target directory with
# --no-index, imported, its RECORD checked, and listed by pip list --path.
echo "== python3-pip =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pip.XXXXXX") || { echo "FAIL python3-pip (no temporary directory)"; exit 1; }
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
import subprocess, json, zipfile, hashlib, base64
env = dict(os.environ, PIP_DISABLE_PIP_VERSION_CHECK="1", PIP_NO_CACHE_DIR="1", PIP_NO_INPUT="1", HOME=os.getcwd(), PYTHONNOUSERSITE="1")
def pip(*args):
    p = subprocess.run([sys.executable, "-m", "pip"] + list(args), capture_output=True, text=True, env=env)
    if p.returncode: info("pip %s: rc %d: %s" % (" ".join(args), p.returncode, (p.stderr or p.stdout).strip().splitlines()[-1:] ))
    return p
def version():
    p = pip("--version")
    info(p.stdout.strip())
    check(p.returncode == 0 and p.stdout.startswith("pip 25.1.1 from ") and "python 3.14" in p.stdout, "pip --version: pip 25.1.1, python 3.14")
    p = pip("list", "--format=json")
    pk = {d["name"].lower(): d["version"] for d in json.loads(p.stdout or "[]")}
    check(pk.get("pip") == "25.1.1", "pip list (JSON) includes pip 25.1.1 (%d distributions)" % len(pk))
    p = pip("show", "pip")
    check(p.returncode == 0 and "Name: pip" in p.stdout and "Location: " in p.stdout, "pip show pip")
    open("f.bin", "wb").write(b"qnx" * 1000)
    p = pip("hash", "f.bin")
    check(hashlib.sha256(b"qnx" * 1000).hexdigest() in p.stdout, "pip hash: sha256")
def install():
    files = {"qnxdemo/__init__.py": b"ANSWER = 42\ndef greet(n):\n    return 'hello ' + n\n",
             "qnxdemo-0.1.dist-info/METADATA": b"Metadata-Version: 2.1\nName: qnxdemo\nVersion: 0.1\nSummary: QNX pip test\n",
             "qnxdemo-0.1.dist-info/WHEEL": b"Wheel-Version: 1.0\nGenerator: qnx-port-test\nRoot-Is-Purelib: true\nTag: py3-none-any\n"}
    rec = []
    for n, d in files.items():
        h = base64.urlsafe_b64encode(hashlib.sha256(d).digest()).rstrip(b"=").decode()
        rec.append("%s,sha256=%s,%d" % (n, h, len(d)))
    rec.append("qnxdemo-0.1.dist-info/RECORD,,")
    with zipfile.ZipFile("qnxdemo-0.1-py3-none-any.whl", "w") as z:
        for n, d in files.items(): z.writestr(n, d)
        z.writestr("qnxdemo-0.1.dist-info/RECORD", "\n".join(rec) + "\n")
    p = pip("install", "--no-index", "--no-deps", "--target", "site", "qnxdemo-0.1-py3-none-any.whl")
    check(p.returncode == 0 and os.path.isfile("site/qnxdemo/__init__.py"), "pip install --no-index --target site qnxdemo-0.1 wheel")
    r = subprocess.run([sys.executable, "-c", "import qnxdemo; print(qnxdemo.ANSWER, qnxdemo.greet('qnx'))"], capture_output=True, text=True, env=dict(env, PYTHONPATH="site"))
    check(r.stdout.strip() == "42 hello qnx", "the installed package imports: 42 hello qnx")
    recs = open("site/qnxdemo-0.1.dist-info/RECORD").read()
    check("qnxdemo/__init__.py,sha256=" in recs and os.path.isfile("site/qnxdemo-0.1.dist-info/INSTALLER"), "RECORD and INSTALLER written")
    p = pip("list", "--path", "site", "--format=json")
    check({"name": "qnxdemo", "version": "0.1"} in json.loads(p.stdout or "[]"), "pip list --path site shows qnxdemo 0.1")
    p = pip("install", "--no-index", "--target", "site2", "no-such-package-qnx")
    check(p.returncode != 0, "--no-index install of an unknown package fails cleanly")
for n, f in (("version", version), ("install", install)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-pip"; exit 0; }
echo "FAIL python3-pip (rc=$rc)"; exit 1
