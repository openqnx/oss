#!/bin/sh
# ml_dtypes on the ported numpy: the machine-learning number formats with
# known answers -- bfloat16 rounding and limits, the float8 variants (E4M3FN
# has no infinity, E5M2 does), the float4/float6 value sets, int4/uint4
# ranges and wrap-around, casts between them, and numpy arithmetic, ufuncs
# and reductions on arrays of these dtypes (the C++ extension's casts and
# ufunc loops; patches/0001 is about its Eigen::half/bfloat16 helpers).
echo "== python3-ml-dtypes =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-mldtypes.XXXXXX") || { echo "FAIL python3-ml-dtypes (no temporary directory)"; exit 1; }
cd "$T" || exit 1
cat > t.py <<'P'
import sys, math, warnings
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
warnings.simplefilter("ignore")
import numpy as np
import ml_dtypes as md
info("ml_dtypes %s, numpy %s" % (md.__version__, np.__version__))
def bf16():
    a = np.array([1.5, 3.14159, 1e-3, 65504.0], dtype=md.bfloat16)
    check(a.astype(np.float32).tolist() == [1.5, 3.140625, 0.00099945068359375, 65536.0], "bfloat16 rounding: 3.14159 -> 3.140625, 65504 -> 65536")
    fi = md.finfo(md.bfloat16)
    check(float(fi.eps) == 2.0 ** -7 and float(fi.max) == 3.3895313892515355e38, "finfo(bfloat16): eps 2^-7, max 3.3895e38")
    s = (np.ones(256, dtype=md.bfloat16) * md.bfloat16(0.5)).astype(np.float32).sum()
    check(s == 128.0, "bfloat16 array multiply: 256 x 0.5 sums to 128")
    check(np.isinf(np.array([3.4e38], dtype=np.float32).astype(md.bfloat16)[0] * md.bfloat16(2)), "bfloat16 overflow to inf")
    check(str(a.dtype) == "bfloat16" and a.itemsize == 2, "dtype name bfloat16, 2 bytes")
def fp8():
    e4 = md.float8_e4m3fn; e5 = md.float8_e5m2
    check(float(md.finfo(e4).max) == 448.0 and float(md.finfo(e5).max) == 57344.0, "float8 max: e4m3fn 448, e5m2 57344")
    x = np.array([np.inf, 1000.0], dtype=np.float32)
    y4 = x.astype(e4).astype(np.float32); y5 = x.astype(e5).astype(np.float32)
    check(np.isnan(y4[0]) and np.isinf(y5[0]), "inf: e4m3fn has none (nan), e5m2 keeps it")
    check(float(np.array([0.3], dtype=np.float32).astype(e4)[0]) == 0.3125, "e4m3fn(0.3) = 0.3125")
    check(float(md.finfo(md.float8_e4m3b11fnuz).max) == 30.0 and float(md.finfo(md.float8_e5m2fnuz).max) == 57344.0, "fnuz variants: e4m3b11fnuz max 30")
    v = np.arange(-4, 5, dtype=np.float32).astype(md.float8_e4m3fn)
    check((v + v).astype(np.float32).tolist() == [-8, -6, -4, -2, 0, 2, 4, 6, 8], "float8 array addition")
def fp4_6():
    vals = sorted(set(np.arange(16, dtype=np.uint8).view(md.float4_e2m1fn).astype(np.float32).tolist()))
    check(vals == [-6, -4, -3, -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 3, 4, 6], "float4_e2m1fn: the 15 representable values")
    check(float(md.finfo(md.float6_e2m3fn).max) == 7.5 and float(md.finfo(md.float6_e3m2fn).max) == 28.0, "float6 max: e2m3fn 7.5, e3m2fn 28")
def ints():
    i4 = md.int4; u4 = md.uint4
    check(int(md.iinfo(i4).min) == -8 and int(md.iinfo(i4).max) == 7 and int(md.iinfo(u4).max) == 15, "int4 -8..7, uint4 0..15")
    a = np.array([7, -8, 3], dtype=i4)
    check((a + np.array([1, -1, 4], dtype=i4)).astype(np.int32).tolist() == [-8, 7, 7], "int4 addition: 7+1 and -8-1 wrap around, 3+4 = 7")
    check(np.array([1, 2, 3, 4], dtype=u4).astype(np.int32).sum() == 10 and np.array([5], dtype=u4).itemsize == 1, "uint4 sum, 1 byte per element")
    check(int(md.iinfo(md.int2).min) == -2 and int(md.iinfo(md.uint2).max) == 3, "int2 -2..1, uint2 0..3")
def casts():
    a = np.array([0.1, 0.5, 2.0], dtype=np.float64)
    b = a.astype(md.bfloat16).astype(md.float8_e5m2).astype(np.float64)
    check(b.tolist() == [0.09375, 0.5, 2.0], "float64 -> bfloat16 -> float8_e5m2 -> float64")
    check(np.array([2.6, -3.4], dtype=md.bfloat16).astype(md.int4).astype(int).tolist() == [2, -3], "bfloat16 -> int4 truncates")
def ufuncs():
    a = np.array([1.0, 4.0, 9.0, -1.0], dtype=md.bfloat16)
    check(np.sqrt(a[:3]).astype(np.float32).tolist() == [1.0, 2.0, 3.0], "sqrt on bfloat16")
    check(bool(np.isnan(np.sqrt(a[3:]))[0]), "sqrt(-1) is nan in bfloat16")
    check(float(np.max(a)) == 9.0 and float(np.min(a)) == -1.0 and int(np.argmax(a)) == 2, "max, min, argmax")
    m = np.arange(6, dtype=np.float32).reshape(2, 3).astype(md.bfloat16)
    check((m @ m.T).astype(np.float32).tolist() == [[5.0, 14.0], [14.0, 50.0]], "bfloat16 matrix product")
    check(np.nextafter(md.bfloat16(1), md.bfloat16(2)) == md.bfloat16(1.0078125), "nextafter(1, 2) = 1.0078125")
for n, f in (("bfloat16", bf16), ("float8", fp8), ("float4/6", fp4_6), ("int4", ints), ("casts", casts), ("ufuncs", ufuncs)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS python3-ml-dtypes"; exit 0; }
echo "FAIL python3-ml-dtypes (rc=$rc)"; exit 1
