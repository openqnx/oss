#!/bin/sh
# python3-setuptools: build a small project the way setup.py / PEP 517
# builds do -- find_packages, package data, metadata queries, build_py into
# build/lib, an sdist and a wheel (setuptools' own bdist_wheel) whose
# contents and metadata are checked; plus pkg_resources-style requirement
# parsing.
echo "== python3-setuptools =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-setuptools.XXXXXX") || { echo "FAIL python3-setuptools (no temporary directory)"; exit 1; }
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
import subprocess, zipfile, tarfile, warnings, importlib.metadata as md
warnings.simplefilter("ignore")
import setuptools
info("setuptools %s" % setuptools.__version__)
os.makedirs("proj/qnxdemo/sub"); os.chdir("proj")
open("qnxdemo/__init__.py", "w").write("__version__ = '1.2.3'\n")
open("qnxdemo/sub/__init__.py", "w").write("")
open("qnxdemo/data.txt", "w").write("payload\n")
open("setup.py", "w").write("""from setuptools import setup, find_packages
setup(name="qnxdemo", version="1.2.3", description="QNX setuptools test",
      packages=find_packages(), package_data={"qnxdemo": ["data.txt"]},
      install_requires=["packaging>=20"], python_requires=">=3.8")
""")
def py(*a):
    p = subprocess.run([sys.executable, "setup.py", "-q"] + list(a), capture_output=True, text=True,
                       env=dict(os.environ, DISTUTILS_DEBUG="1"))     # tracebacks, not just "error: ..."
    if p.returncode: info("setup.py %s: rc %d: %s" % (" ".join(a), p.returncode, (p.stderr or p.stdout).strip()[-1200:]))
    return p
def meta():
    check(md.version("setuptools") == setuptools.__version__ == "80.8.0", "setuptools 80.8.0 (importlib.metadata agrees)")
    check(sorted(setuptools.find_packages()) == ["qnxdemo", "qnxdemo.sub"], "find_packages: qnxdemo, qnxdemo.sub")
    p = subprocess.run([sys.executable, "setup.py", "--name", "--version"], capture_output=True, text=True)
    check(p.stdout.split()[-2:] == ["qnxdemo", "1.2.3"], "setup.py --name --version")
def stdlib():
    # bdist_wheel turns the egg-info into dist-info and removes it with
    # shutil.rmtree: the stdlib's fd-based tree walk must work here.
    import shutil
    os.makedirs("rmt/a/./b/c"); open("rmt/a/b/c/f", "w").write("x"); open("rmt/a/g", "w").write("y")
    try: shutil.rmtree("rmt/a/./b"); shutil.rmtree("rmt"); ok = not os.path.exists("rmt")
    except OSError as e: ok = False; info("shutil.rmtree: %r (CPython's fd-based rmtree, the python3 port)" % e)
    check(ok, "shutil.rmtree removes a directory tree (used by bdist_wheel)")
def build():
    p = py("build")
    check(p.returncode == 0 and os.path.isfile("build/lib/qnxdemo/sub/__init__.py") and os.path.isfile("build/lib/qnxdemo/data.txt"),
          "setup.py build: packages and package data in build/lib")
def dist():
    p = py("sdist", "--formats=gztar")
    sd = "dist/qnxdemo-1.2.3.tar.gz"
    names = tarfile.open(sd).getnames() if os.path.isfile(sd) else []
    check(p.returncode == 0 and "qnxdemo-1.2.3/setup.py" in names and "qnxdemo-1.2.3/PKG-INFO" in names, "sdist qnxdemo-1.2.3.tar.gz with setup.py, PKG-INFO")
    p = py("bdist_wheel")
    wh = "dist/qnxdemo-1.2.3-py3-none-any.whl"
    ok = p.returncode == 0 and os.path.isfile(wh)
    check(ok, "bdist_wheel: qnxdemo-1.2.3-py3-none-any.whl")
    if ok:
        z = zipfile.ZipFile(wh); n = z.namelist()
        check("qnxdemo/data.txt" in n and "qnxdemo-1.2.3.dist-info/RECORD" in n, "wheel holds package data and RECORD")
        m = z.read("qnxdemo-1.2.3.dist-info/METADATA").decode()
        check("Name: qnxdemo" in m and "Requires-Dist: packaging>=20" in m and "Requires-Python: >=3.8" in m, "METADATA: name, Requires-Dist, Requires-Python")
        check("Tag: py3-none-any" in z.read("qnxdemo-1.2.3.dist-info/WHEEL").decode(), "WHEEL tag py3-none-any")
def reqs():
    import pkg_resources
    r = pkg_resources.Requirement.parse("Foo-Bar[x]>=1.0,!=1.3")
    check(r.project_name == "Foo-Bar" and r.extras == ("x",) and "1.2" in r and "1.3" not in r, "pkg_resources.Requirement.parse")
    check(pkg_resources.parse_version("1.10") > pkg_resources.parse_version("1.9"), "parse_version ordering")
for n, f in (("metadata", meta), ("stdlib", stdlib), ("build", build), ("dist", dist), ("pkg_resources", reqs)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-setuptools"; exit 0; }
echo "FAIL python3-setuptools (rc=$rc)"; exit 1
