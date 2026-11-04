#!/bin/sh
# python3-cffi and python3-cparser: cffi in ABI mode calls libc through
# libffi with no compiler -- strlen, abs, snprintf (varargs), qsort with a
# Python callback, a struct passed by pointer (div) -- and allocates and
# casts C data; pycparser (which cffi's cdef uses) parses a C translation
# unit into the expected AST and regenerates C from it.
echo "== python3-cffi =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-cffi.XXXXXX") || { echo "FAIL python3-cffi (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
import cffi, _cffi_backend, pycparser
info("cffi %s, pycparser %s, backend %s" % (cffi.__version__, pycparser.__version__, _cffi_backend.__file__))
check(_cffi_backend.__file__.endswith(".so"), "_cffi_backend is the compiled extension")
ffi = cffi.FFI()
ffi.cdef("""
    size_t strlen(const char *);
    int abs(int);
    int snprintf(char *, size_t, const char *, ...);
    typedef int (*cmp_t)(const void *, const void *);
    void qsort(void *, size_t, size_t, cmp_t);
    typedef struct { int quot; int rem; } div_t;
    div_t div(int, int);
""")
C = ffi.dlopen(None)
check(C.strlen(b"QNX Neutrino") == 12, "strlen() through ABI mode")
check(C.abs(-42) == 42, "abs()")
buf = ffi.new("char[64]")
n = C.snprintf(buf, 64, b"%d-%s-%.2f", ffi.cast("int", 7), ffi.new("char[]", b"qnx"), ffi.cast("double", 3.14159))
check(n == 10 and ffi.string(buf) == b"7-qnx-3.14", "snprintf() with varargs")
@ffi.callback("int(const void *, const void *)")
def cmp(a, b):
    x = ffi.cast("int *", a)[0]; y = ffi.cast("int *", b)[0]
    return (x > y) - (x < y)
arr = ffi.new("int[]", [5, 3, 9, 1, 7, 2])
C.qsort(arr, 6, ffi.sizeof("int"), cmp)
check(list(arr) == [1, 2, 3, 5, 7, 9], "qsort() with a Python callback")
d = C.div(17, 5)
check(d.quot == 3 and d.rem == 2, "div() returns a struct by value")
check(ffi.sizeof("div_t") == 8 and ffi.typeof(d).cname == "div_t", "struct size and type name")
p = ffi.new("int *", 123)
check(p[0] == 123 and int(ffi.cast("intptr_t", p)) != 0, "ffi.new allocates, ffi.cast to integer")
try:
    ffi.cdef("int this is not C;"); check(False, "bad cdef raises CDefError")
except cffi.CDefError:
    check(True, "bad cdef raises CDefError")
from pycparser import c_parser, c_generator, c_ast
src = "typedef unsigned int u32; struct pt { int x, y; }; static int add(int a, int b) { return a + b * 2; }"
ast = c_parser.CParser().parse(src)
names = [type(e).__name__ for e in ast.ext]
check(names == ["Typedef", "Decl", "FuncDef"], "pycparser: typedef, struct, function definition")
fn = ast.ext[2]
check(fn.decl.name == "add" and [p.name for p in fn.decl.type.args.params] == ["a", "b"], "pycparser: function name and parameters")
ret = fn.body.block_items[0].expr
check(isinstance(ret, c_ast.BinaryOp) and ret.op == "+" and ret.right.op == "*", "pycparser: operator precedence in the AST")
gen = c_generator.CGenerator().visit(fn)
check("return a + (b * 2);" in gen, "pycparser: C regenerated from the AST")
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-cffi"; exit 0; }
echo "FAIL python3-cffi (rc=$rc)"; exit 1
