#!/bin/sh
# numpy on the ported CPython: linear algebra with known answers (solve,
# det, inverse, eigenvalues, SVD, least squares), FFTs, broadcasting and
# reductions, integer and float16 dtypes, a seeded random generator's
# statistics and reproducibility, and an array saved to and loaded from .npy.
echo "== numpy =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-numpy.XXXXXX") || { echo "FAIL numpy (no temporary directory)"; exit 1; }
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
import numpy as np
info("numpy %s, python %s" % (np.__version__, sys.version.split()[0]))
def linalg():
    A = np.array([[3.0, 1.0], [1.0, 2.0]]); b = np.array([9.0, 8.0])
    check(np.allclose(np.linalg.solve(A, b), [2.0, 3.0]), "solve [[3,1],[1,2]] x = [9,8] -> [2,3]")
    check(abs(np.linalg.det(np.array([[1.0, 2.0], [3.0, 4.0]])) + 2.0) < 1e-12, "det [[1,2],[3,4]] = -2")
    M = np.array([[4.0, 7.0], [2.0, 6.0]])
    check(np.allclose(np.linalg.inv(M) @ M, np.eye(2)), "inv(M) @ M = I")
    w = np.linalg.eigvalsh(np.array([[2.0, 1.0], [1.0, 2.0]]))
    check(np.allclose(sorted(w), [1.0, 3.0]), "eigenvalues of [[2,1],[1,2]] = 1, 3")
    s = np.linalg.svd(np.array([[3.0, 0.0], [0.0, -4.0]]), compute_uv=False)
    check(np.allclose(s, [4.0, 3.0]), "singular values of diag(3,-4) = 4, 3")
    x = np.arange(10.0); y = 2.5 * x - 1.0
    coef, *_ = np.linalg.lstsq(np.vstack([x, np.ones_like(x)]).T, y, rcond=None)
    check(np.allclose(coef, [2.5, -1.0]), "least squares fit of y = 2.5x - 1")
    R = np.random.default_rng(7).standard_normal((60, 60))
    check(np.allclose(R @ np.linalg.inv(R), np.eye(60), atol=1e-8), "60x60 random matrix times its inverse = I")
    q, r = np.linalg.qr(R); check(np.allclose(q @ r, R), "QR decomposition reproduces the matrix")
    try:
        cfg = np.show_config(mode="dicts"); info("BLAS: %s" % cfg.get("Build Dependencies", {}).get("blas", {}).get("name", "?"))
    except Exception as e: info("show_config: %r" % e)
def fft():
    check(np.allclose(np.fft.fft([1, 0, 0, 0]), [1, 1, 1, 1]), "fft of an impulse is flat")
    check(np.allclose(np.fft.fft([0, 1, 0, -1]), [0, -2j, 0, 2j]), "fft [0,1,0,-1] = [0,-2j,0,2j]")
    t = np.arange(256); sig = np.sin(2 * np.pi * 10 * t / 256)
    check(int(np.argmax(np.abs(np.fft.rfft(sig)))) == 10, "a 10-cycle sine peaks in bin 10")
    check(np.allclose(np.fft.ifft(np.fft.fft(sig)).real, sig), "ifft(fft(x)) = x")
def arrays():
    a = np.arange(10).reshape(2, 5)
    check(a.sum(axis=1).tolist() == [10, 35] and a.T.shape == (5, 2), "reshape, transpose, sum over an axis")
    check((np.ones((3, 1)) * np.arange(4)).shape == (3, 4), "broadcasting (3,1) x (4,) -> (3,4)")
    check(np.array([100, 100, 100], dtype=np.int8).sum(dtype=np.int64) == 300, "int8 array summed as int64: 300")
    check((np.array([250], dtype=np.uint8) + np.uint8(10))[0] == 4, "uint8 arrays wrap around: 250 + 10 = 4")
    check(np.float16(65504) == 65504 and np.isinf(np.float16(65520)), "float16: max 65504, 65520 overflows")
    check(np.einsum("ij,jk->ik", [[1, 2], [3, 4]], [[5, 6], [7, 8]]).tolist() == [[19, 22], [43, 50]], "einsum matrix product")
    s = np.sort(np.array([3, 1, 2])); check(s.tolist() == [1, 2, 3] and np.searchsorted(s, 2) == 1, "sort, searchsorted")
    check(np.median([5, 3, 1, 4, 2]) == 3 and np.isclose(np.std([2, 4, 4, 4, 5, 5, 7, 9]), 2.0), "median, std")
    st = np.array([(1, 2.5, b"qnx")], dtype=[("i", "i4"), ("f", "f8"), ("s", "S3")])
    check(st["s"][0] == b"qnx" and st.dtype.itemsize == 15, "structured dtype")
def rng():
    g = np.random.default_rng(12345); x = g.standard_normal(200000)
    check(abs(x.mean()) < 0.01 and abs(x.std() - 1) < 0.01, "standard normal: mean 0, std 1 (200k samples)")
    a = np.random.default_rng(42).integers(0, 1000, 20); b = np.random.default_rng(42).integers(0, 1000, 20)
    check((a == b).all(), "same seed, same stream")
    u = np.random.default_rng(1).random(100000); check(0.49 < u.mean() < 0.51 and u.min() >= 0 and u.max() < 1, "uniform [0,1)")
def io():
    a = np.linspace(0, 1, 11).reshape(11, 1) * np.arange(3)
    np.save("a.npy", a); b = np.load("a.npy")
    check(b.shape == (11, 3) and np.array_equal(a, b), "np.save / np.load round trip")
    np.savetxt("a.csv", a, delimiter=","); check(np.allclose(np.loadtxt("a.csv", delimiter=","), a), "savetxt / loadtxt")
for n, f in (("linalg", linalg), ("fft", fft), ("arrays", arrays), ("random", rng), ("io", io)):
    run(n, f)
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
cd /; rm -rf "$T"
[ $rc -eq 0 ] && { echo "PASS numpy"; exit 0; }
echo "FAIL numpy (rc=$rc)"; exit 1
