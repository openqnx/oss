#!/bin/sh
# meson on the ported CPython, without a compiler: configure a project that
# needs none (the "none" backend) -- options from meson.options, a
# configure_file with substitutions, run_command of a Python script, string,
# array and dict operations -- then read it back with meson introspect,
# change an option with meson configure, and check the reconfigured result.
echo "== meson =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-meson.XXXXXX") || { echo "FAIL meson (no temporary directory)"; exit 1; }
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
import subprocess, json, shutil
M = [sys.executable, "/usr/local/bin/meson"] if os.path.isfile("/usr/local/bin/meson") else [shutil.which("meson") or "meson"]
def meson(*a):
    p = subprocess.run(M + list(a), capture_output=True, text=True)
    if p.returncode: info("meson %s: rc %d:\n%s" % (" ".join(a), p.returncode, (p.stdout + p.stderr)[-1500:]))
    return p
os.makedirs("src")
open("src/meson.build", "w").write("""project('qnxdemo', version : '1.2.3', meson_version : '>= 1.0')
py = import('python').find_installation()
r = run_command(py, '-c', 'print(6 * 7)', check : true)
answer = r.stdout().strip()
assert(answer == '42', 'run_command')
words = 'alpha beta gamma'.split()
d = {'os' : host_machine.system(), 'n' : words.length()}
conf = configuration_data()
conf.set('ANSWER', answer)
conf.set_quoted('FLAVOUR', get_option('flavour'))
conf.set('NWORDS', d['n'])
conf.set10('HAVE_QNX', host_machine.system() == 'qnx')
configure_file(output : 'config.h', configuration : conf)
configure_file(input : 'greeting.in', output : 'greeting.txt', configuration : {'WHO' : '-'.join(words).to_upper()})
message('system=' + host_machine.system() + ' cpu=' + host_machine.cpu_family())
summary({'flavour' : get_option('flavour')})
""")
open("src/meson.options", "w").write("option('flavour', type : 'combo', choices : ['plain', 'spicy'], value : 'plain')\n")
open("src/greeting.in", "w").write("hello @WHO@\n")
def setup():
    p = meson("--version"); info("meson %s" % p.stdout.strip())
    check(p.returncode == 0 and p.stdout.strip() == "1.10.0", "meson --version: 1.10.0")
    p = meson("setup", "--backend=none", "build", "src")
    out = p.stdout
    for l in out.splitlines():
        if "system=" in l or "Build machine cpu" in l: info(l.strip())
    check(p.returncode == 0, "meson setup --backend=none (no compiler needed)")
    h = open("build/config.h").read() if os.path.isfile("build/config.h") else ""
    check("#define ANSWER 42" in h and '#define FLAVOUR "plain"' in h and "#define NWORDS 3" in h, "config.h: ANSWER 42 (run_command), FLAVOUR \"plain\", NWORDS 3")
    check(os.path.isfile("build/greeting.txt") and open("build/greeting.txt").read() == "hello ALPHA-BETA-GAMMA\n", "configure_file substitution: hello ALPHA-BETA-GAMMA")
def introspect():
    p = meson("introspect", "--projectinfo", "build")
    pi = json.loads(p.stdout or "{}")
    check(pi.get("descriptive_name") == "qnxdemo" and pi.get("version") == "1.2.3", "introspect --projectinfo: qnxdemo 1.2.3")
    p = meson("introspect", "--buildoptions", "build")
    opts = {o["name"]: o for o in json.loads(p.stdout or "[]")}
    check(opts.get("flavour", {}).get("value") == "plain" and opts["flavour"].get("choices") == ["plain", "spicy"], "introspect --buildoptions: flavour plain of plain/spicy")
    check(opts.get("buildtype", {}).get("value") == "debug", "default buildtype debug")
def reconfigure():
    p = meson("configure", "build", "-Dflavour=spicy")
    check(p.returncode == 0, "meson configure -Dflavour=spicy")
    p = meson("setup", "--reconfigure", "build", "src")
    h = open("build/config.h").read() if os.path.isfile("build/config.h") else ""
    check(p.returncode == 0 and '#define FLAVOUR "spicy"' in h, "reconfigured: config.h has FLAVOUR \"spicy\"")
    p = meson("configure", "build", "-Dflavour=sweet")
    check(p.returncode != 0, "a value outside the combo is rejected")
for n, f in (("setup", setup), ("introspect", introspect), ("reconfigure", reconfigure)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS meson"; exit 0; }
echo "FAIL meson (rc=$rc)"; exit 1
