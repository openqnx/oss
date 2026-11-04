#!/bin/sh
# python3-virtualenv: creates environments offline (seeded from the wheels
# virtualenv embeds): the environment's python runs with sys.prefix in the
# environment and sys.base_prefix in the installation; its pip is the
# environment's own and installs a local package into it; the
# environment does not see the installation's site-packages, unless
# created with --system-site-packages; bin/activate puts it first in PATH;
# --without-pip makes one with no pip. (virtualenv's own dependencies
# filelock, platformdirs and distlib run along.)
echo "== python3-virtualenv =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-venv.XXXXXX") || { echo "FAIL python3-virtualenv (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
T=$(pwd -P)   # /tmp is a symbolic link on QNX images; virtualenv resolves it
export HOME="$T" PIP_NO_INDEX=1 PIP_DISABLE_PIP_VERSION_CHECK=1 VIRTUALENV_OVERRIDE_APP_DATA="$T/appdata"
fail=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=1; }
"$PY" -m virtualenv --version | sed 's/^/  info /'
"$PY" -m virtualenv --no-download --no-periodic-update -q env1 > v1.log 2>&1; rc=$?
[ $rc -eq 0 ] && [ -x env1/bin/python ] && ok "virtualenv env1 created (rc 0, bin/python)" || { bad "virtualenv env1 created (rc $rc)"; tail -n 20 v1.log | sed 's/^/  | /'; }
r=$(env1/bin/python -c "import sys, os; print(os.path.realpath(sys.prefix) == os.path.realpath('$T/env1'), sys.prefix != sys.base_prefix, sys.version_info[:2] == (3, 14))" 2>&1)
[ "$r" = "True True True" ] && ok "env python: sys.prefix is env1, base_prefix the installation, Python 3.14" || bad "env python prefixes ($r)"
r=$(env1/bin/python -m pip --version 2>&1)
case "$r" in "pip "*"$T/env1/"*) ok "env pip is the environment's own: ${r%% from*}" ;; *) bad "env pip ($r)" ;; esac
r=$(env1/bin/python -c "import filelock" >/dev/null 2>&1 && echo visible || echo isolated)
[ "$r" = isolated ] && ok "env does not see the installation's site-packages (filelock)" || bad "env isolation ($r)"
# For Python >= 3.12 virtualenv seeds pip alone (no setuptools to build a
# source tree): the package is a wheel, made here by hand.
"$PY" - <<'P'
import zipfile, base64, hashlib
files = {"qnxv/__init__.py": b"N = 7\n",
         "qnxv-0.7.dist-info/METADATA": b"Metadata-Version: 2.1\nName: qnxv\nVersion: 0.7\n",
         "qnxv-0.7.dist-info/WHEEL": b"Wheel-Version: 1.0\nGenerator: hand\nRoot-Is-Purelib: true\nTag: py3-none-any\n"}
rec = "".join("%s,sha256=%s,%d\n" % (n, base64.urlsafe_b64encode(hashlib.sha256(c).digest()).rstrip(b"=").decode(), len(c)) for n, c in files.items())
with zipfile.ZipFile("qnxv-0.7-py3-none-any.whl", "w") as z:
    for n, c in files.items(): z.writestr(n, c)
    z.writestr("qnxv-0.7.dist-info/RECORD", rec + "qnxv-0.7.dist-info/RECORD,,\n")
P
env1/bin/python -m pip install -q ./qnxv-0.7-py3-none-any.whl > pip.log 2>&1
r=$(env1/bin/python -c "import qnxv, importlib.metadata as m; print(qnxv.N, m.version('qnxv'))" 2>&1)
[ "$r" = "7 0.7" ] && ok "env pip installs a local wheel into the environment" || { bad "env pip install ($r)"; tail -n 10 pip.log | sed 's/^/  | /'; }
r=$("$PY" -c "import qnxv" >/dev/null 2>&1 && echo leaked || echo kept)
[ "$r" = kept ] && ok "the package stays in the environment (the installation does not see it)" || bad "package leaked into the installation"
r=$(sh -c ". env1/bin/activate && command -v python && echo \$VIRTUAL_ENV" 2>&1 | tr '\n' ' ')
[ "$r" = "$T/env1/bin/python $T/env1 " ] && ok "bin/activate: python and VIRTUAL_ENV point at env1" || bad "bin/activate ($r)"
"$PY" -m virtualenv --no-download --no-periodic-update -q --system-site-packages --without-pip env2 > v2.log 2>&1 || { bad "virtualenv env2"; tail -n 10 v2.log | sed 's/^/  | /'; }
r=$(env2/bin/python -c "import filelock, os; print(filelock.FileLock.__module__, os.path.dirname(os.path.dirname(filelock.__file__)))" 2>&1)
[ "$r" = "filelock._unix $("$PY" -c 'import sysconfig; print(sysconfig.get_paths()["purelib"])')" ] && ok "--system-site-packages: the installation's filelock is visible" || bad "--system-site-packages ($r)"
[ ! -e env2/lib/python3.14/site-packages/pip ] && [ -x env2/bin/python ] && ok "--without-pip: no pip in env2" || bad "--without-pip: env2 has its own pip"
[ $fail -eq 0 ] && { echo "PASS python3-virtualenv"; exit 0; }
echo "FAIL python3-virtualenv"; exit 1
