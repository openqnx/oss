#!/bin/sh
# The Python build tools of group g16, each driving a fake compiler (no C
# compiler on the target):
#  - scons: an SConstruct with two Builders around a fake "compiler" and
#    "linker" (Python scripts) builds app from two sources, the second run
#    is "up to date", and touching one source rebuilds exactly that object;
#  - gyp: a .gyp project becomes Ninja files and a Makefile naming the
#    sources, defines and the executable;
#  - pythran: translates a numpy kernel with an export signature to C++
#    (pythran -E) and pythran-config names its own headers.
echo "== python3-buildtools =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
BIN=$(dirname "$PY")
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-buildtools.XXXXXX") || { echo "FAIL python3-buildtools (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
# No __pycache__ in the installation's site-packages (docs/porting-hazards.md
# 183), but bytecode in a private cache under $T: this test starts Python
# many times, and recompiling flake8/SCons/pythran from source on every
# start took it past the 15-minute limit on the emulated aarch64 VM.
unset PYTHONDONTWRITEBYTECODE; export PYTHONPYCACHEPREFIX="$T/pycache"
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails+1)); }
tool() { [ -x "$BIN/$1" ] && echo "$BIN/$1" || echo "$1"; }
# ---- scons
mkdir sc && cd sc || exit 1
cat > fakecc.py <<'P'
import sys
src, out = sys.argv[1:3]
with open(out, "w") as f:
    f.write("OBJ " + open(src).read().strip().upper() + "\n")
with open("log", "a") as f:
    f.write("cc " + src + "\n")
P
cat > fakeld.py <<'P'
import sys
out, objs = sys.argv[1], sys.argv[2:]
with open(out, "w") as f:
    f.write("".join(open(o).read() for o in objs))
with open("log", "a") as f:
    f.write("ld " + out + "\n")
P
cat > SConstruct <<'P'
import os, sys
env = Environment(ENV=os.environ, PY=sys.executable)
env.Append(BUILDERS={
    "FakeCC": Builder(action="$PY fakecc.py $SOURCE $TARGET", suffix=".o", src_suffix=".c"),
    "FakeLD": Builder(action="$PY fakeld.py $TARGET $SOURCES"),
})
objs = [env.FakeCC(s) for s in ("hello.c", "world.c")]
env.FakeLD("app", objs)
P
echo "hello" > hello.c; echo "world" > world.c
SCONS=$(tool scons)
"$SCONS" -Q > o1 2>&1; rc=$?
[ $rc -eq 0 ] && [ "$(cat app 2>/dev/null)" = "OBJ HELLO
OBJ WORLD" ] && ok "scons builds app from two sources with the fake compiler" \
  || { bad "scons builds app (rc=$rc)"; sed 's/^/  info /' o1; }
[ "$(sort log | tr '\n' ' ')" = "cc hello.c cc world.c ld app " ] && ok "scons ran each step once" || { bad "scons ran each step once"; sed 's/^/  info /' log; }
: > log; "$SCONS" -Q > o2 2>&1
grep -q "is up to date" o2 && [ ! -s log ] && ok "second scons run: up to date, nothing run" || { bad "second scons run: up to date"; sed 's/^/  info /' o2; }
echo "hello again" > hello.c; : > log; "$SCONS" -Q > o3 2>&1
[ "$(sort log | tr '\n' ' ')" = "cc hello.c ld app " ] && grep -q "OBJ HELLO AGAIN" app && ok "a changed source rebuilds exactly its object and the link" \
  || { bad "a changed source rebuilds exactly its object"; sed 's/^/  info /' log; }
"$SCONS" -Q -c > o4 2>&1; [ ! -e app ] && [ ! -e hello.o ] && ok "scons -c removes the targets" || bad "scons -c removes the targets"
cd "$T" || exit 1
# ---- gyp
mkdir gy && cd gy || exit 1
cat > hello.gyp <<'P'
{
  'targets': [{
    'target_name': 'hello',
    'type': 'executable',
    'defines': ['GREETING="qnx"'],
    'sources': ['main.c', 'util.c'],
  }],
}
P
: > main.c; : > util.c
GYP=$(tool gyp)
"$GYP" --no-parallel --format=ninja --depth=. hello.gyp > g1 2>&1; rc=$?
n=$(ls out/Default/obj/hello.ninja 2>/dev/null)
[ $rc -eq 0 ] && [ -n "$n" ] && grep -q "main.c" "$n" && grep -q "util.c" "$n" && grep -q "GREETING" "$n" \
  && grep -q "build hello:" "$n" && ok "gyp --format=ninja: sources, define and the executable" \
  || { bad "gyp --format=ninja (rc=$rc)"; sed 's/^/  info /' g1; ls -R out 2>/dev/null | sed 's/^/  info /'; }
"$GYP" --no-parallel --format=make --depth=. hello.gyp > g2 2>&1; rc=$?
[ $rc -eq 0 ] && [ -f Makefile ] && grep -q "hello" hello.target.mk 2>/dev/null && grep -q "util.o" hello.target.mk \
  && ok "gyp --format=make: Makefile and hello.target.mk" || { bad "gyp --format=make (rc=$rc)"; sed 's/^/  info /' g2; ls | sed 's/^/  info /'; }
cd "$T" || exit 1
# ---- pythran
mkdir pt && cd pt || exit 1
cat > kern.py <<'P'
#pythran export dprod(float64[], float64[])
import numpy as np
def dprod(a, b):
    return np.sum(a * b)
P
PYTHRAN=$(tool pythran)
"$PYTHRAN" -E kern.py -o kern.cpp > p1 2>&1; rc=$?
[ $rc -eq 0 ] && grep -q "dprod" kern.cpp && grep -q "pythonic/include" kern.cpp && ok "pythran -E translates the kernel to C++" \
  || { bad "pythran -E (rc=$rc)"; sed 's/^/  info /' p1; }
"$PY" -c 'import numpy as np, kern; assert kern.dprod(np.array([1.0, 2.0, 3.0]), np.array([4.0, 5.0, 6.0])) == 32.0' \
  && ok "the kernel itself runs as Python (32.0)" || bad "the kernel itself runs as Python"
PC=$(tool pythran-config)
"$PC" --cflags > p2 2>&1; d=$("$PY" -c 'import os, pythran; print(os.path.dirname(pythran.__file__))')
grep -q -- "-I$d" p2 && [ -f "$d/pythonic/include/numpy/sum.hpp" ] && ok "pythran-config --cflags names the installed headers" \
  || { bad "pythran-config --cflags"; sed 's/^/  info /' p2; }
[ $fails -eq 0 ] && { echo "PASS python3-buildtools"; exit 0; }
echo "FAIL python3-buildtools ($fails)"; exit 1
