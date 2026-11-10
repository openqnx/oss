#!/bin/sh
# Pure-Python libraries of group g16, known answers each: absl-py (flags
# parsed from an argument list, absl.logging), gast and beniget (a portable
# AST and the def-use chains of a small function), bracex (Bash brace
# expansion), cppy (its C++ headers are installed and found by
# get_include()), PyFilesystem2 (a memory filesystem, an OS directory and a
# tar archive written and read back), opt_einsum (the optimal contraction
# path and FLOP count of a matrix chain; a contraction of numpy arrays), py (py.path.local), distro (the
# module answers on QNX, which has no os-release).
echo "== python3-pylibs-b =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pylibs-b.XXXXXX") || { echo "FAIL python3-pylibs-b (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import os, sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)

# absl-py
from absl import flags, logging as alog
F = flags.FlagValues()
flags.DEFINE_integer("count", 3, "how many", flag_values=F)
flags.DEFINE_string("name", "qnx", "who", flag_values=F)
flags.DEFINE_boolean("verbose", False, "talk", flag_values=F)
flags.DEFINE_enum("mode", "fast", ["fast", "slow"], "mode", flag_values=F)
rest = F(["prog", "--count=7", "--noverbose", "--mode", "slow", "extra"])
check((F.count, F.name, F.verbose, F.mode, rest) == (7, "qnx", False, "slow", ["prog", "extra"]), "absl.flags parses integer, string, boolean and enum flags")
try:
    F(["prog", "--count=seven"]); check(False, "absl.flags rejects a bad integer")
except flags.IllegalFlagValueError:
    check(True, "absl.flags rejects a bad integer")
alog.set_verbosity(alog.WARNING)
check(alog.get_verbosity() == alog.WARNING and not alog.level_info(), "absl.logging verbosity")

# gast + beniget
import gast, beniget
src = "def f(a, b):\n    c = a + b\n    if c:\n        c = 2\n    return c\n"
tree = gast.parse(src)
check(isinstance(tree.body[0].args.args[0], gast.Name), "gast: arguments are gast.Name nodes")
du = beniget.DefUseChains(); du.visit(tree)
fn = tree.body[0]
defs = {}
for n in gast.walk(fn):
    if isinstance(n, gast.Name) and isinstance(n.ctx, (gast.Store, gast.Param)):
        defs.setdefault(n.id, []).append(len(du.chains[n].users()))
check(defs == {"a": [1], "b": [1], "c": [2, 1]}, "beniget: def-use chains of a, b and both definitions of c")
if defs != {"a": [1], "b": [1], "c": [2, 1]}: info(repr(defs))
check(gast.unparse(gast.parse("x = [i * 2 for i in range(3)]")).strip() == "x = [i * 2 for i in range(3)]", "gast round trip through unparse")

# bracex
import bracex
check(bracex.expand("file{1..3}.{c,h}") == ["file1.c", "file1.h", "file2.c", "file2.h", "file3.c", "file3.h"], "bracex: sequence and list expansion")
check(bracex.expand("{a,b{1..2}}x") == ["ax", "b1x", "b2x"], "bracex: nested braces")
check(bracex.expand("{08..10..2}") == ["08", "10"], "bracex: zero padded step")

# cppy
import cppy
inc = cppy.get_include()
check(os.path.isfile(os.path.join(inc, "cppy", "cppy.h")), "cppy.get_include() has cppy/cppy.h")
check("ptr" in open(os.path.join(inc, "cppy", "ptr.h")).read(), "cppy/ptr.h is the C++ header")

# PyFilesystem2
import fs, fs.memoryfs, fs.osfs, fs.tarfs, fs.copy
m = fs.memoryfs.MemoryFS()
m.makedirs("a/b"); m.writetext("a/b/hello.txt", "hello qnx\n"); m.writebytes("a/data.bin", bytes(range(256)))
check(sorted(m.walk.files()) == ["/a/b/hello.txt", "/a/data.bin"], "fs: MemoryFS walk")
check(m.getinfo("a/data.bin", namespaces=["details"]).size == 256, "fs: file size from getinfo")
os.mkdir("osdir")
with fs.osfs.OSFS("osdir") as o:
    fs.copy.copy_fs(m, o)
check(open("osdir/a/b/hello.txt").read() == "hello qnx\n", "fs: copy_fs to an OS directory")
with fs.tarfs.TarFS("t.tar.gz", write=True) as t:
    fs.copy.copy_fs(m, t)
with fs.tarfs.TarFS("t.tar.gz") as t:
    check(t.readbytes("a/data.bin") == bytes(range(256)) and t.readtext("a/b/hello.txt") == "hello qnx\n", "fs: TarFS written and read back")

# opt_einsum
import opt_einsum
path, pinfo = opt_einsum.contract_path("ij,jk,kl->il", (10, 1000), (1000, 10), (10, 1000), shapes=True, optimize="optimal")
check(path == [(0, 1), (0, 1)], "opt_einsum: optimal path of a matrix chain")
check(int(pinfo.opt_cost) == 400000, "opt_einsum: optimised FLOP count 400000")
if int(pinfo.opt_cost) != 400000: info(str(pinfo.opt_cost))
import numpy as np
a, b, c = np.arange(6.).reshape(2, 3), np.arange(12.).reshape(3, 4), np.arange(8.).reshape(4, 2)
r = opt_einsum.contract("ij,jk,kl->il", a, b, c)
check(r.tolist() == [[324.0, 422.0], [1008.0, 1304.0]], "opt_einsum.contract with numpy arrays: exact result")

# py
import py
p = py.path.local(os.getcwd()).join("osdir", "a", "b", "hello.txt")
check(p.check(file=1) and p.read() == "hello qnx\n" and p.basename == "hello.txt" and p.ext == ".txt", "py.path.local reads a file")

# distro
import distro
info("distro: id=%r name=%r" % (distro.id(), distro.name()))
check(isinstance(distro.id(), str) and isinstance(distro.info(), dict), "distro answers (no os-release on QNX)")
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-pylibs-b"; exit 0; }
echo "FAIL python3-pylibs-b (rc=$rc)"; exit 1
