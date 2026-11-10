#!/bin/sh
# python3-scipy with known answers: linalg.solve of a 3x3 system (exact
# solution 1, -2, -2) and the LU/eigenvalues of a symmetric matrix (OpenBLAS's
# LAPACK); optimize.minimize of the Rosenbrock function from (-1.2, 1)
# reaches (1, 1); fft of a cosine has its two peaks of n/2 and nothing
# else; integrate.quad of exp(-x^2) over the real line is sqrt(pi);
# special.gamma(5) = 24 and erf; an ODE (solve_ivp, y' = -y) gives exp(-t);
# sparse matrix-vector product; stats.norm.cdf(1.96); interpolation; and a
# Pythran-built module (scipy.optimize._group_columns) loads.
echo "== python3-scipy =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-scipy.XXXXXX") || { echo "FAIL python3-scipy (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import math, sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
import numpy as np, scipy
info("SciPy %s, NumPy %s" % (scipy.__version__, np.__version__))
from scipy import linalg, optimize, fft, integrate, special, sparse, stats, interpolate
A = np.array([[3.0, 2.0, -1.0], [2.0, -2.0, 4.0], [-1.0, 0.5, -1.0]])
x = linalg.solve(A, np.array([1.0, -2.0, 0.0]))
check(np.allclose(x, [1.0, -2.0, -2.0]), "linalg.solve: (1, -2, -2)")
if not np.allclose(x, [1.0, -2.0, -2.0]): info(repr(x))
S = np.array([[2.0, -1.0, 0.0], [-1.0, 2.0, -1.0], [0.0, -1.0, 2.0]])
w = linalg.eigh(S, eigvals_only=True)
check(np.allclose(w, [2 - math.sqrt(2), 2.0, 2 + math.sqrt(2)]), "linalg.eigh: 2-sqrt2, 2, 2+sqrt2")
P_, L, U = linalg.lu(A)
check(np.allclose(P_ @ L @ U, A) and abs(linalg.det(A) - np.linalg.det(A)) < 1e-12, "linalg.lu and det")
r = optimize.minimize(optimize.rosen, [-1.2, 1.0], method="BFGS", jac=optimize.rosen_der)
check(r.success and np.allclose(r.x, [1.0, 1.0], atol=1e-5), "optimize.minimize (BFGS) of Rosenbrock reaches (1, 1)")
r2 = optimize.minimize(optimize.rosen, [-1.2, 1.0], method="L-BFGS-B", bounds=[(-2, 2), (-2, 2)])
check(np.allclose(r2.x, [1.0, 1.0], atol=1e-4), "optimize.minimize (L-BFGS-B, the C translation) reaches (1, 1)")
check(abs(optimize.brentq(lambda t: t * t - 2, 0, 2) - math.sqrt(2)) < 1e-12, "optimize.brentq: sqrt(2)")
n = 64; t = np.arange(n)
X = fft.fft(np.cos(2 * np.pi * 5 * t / n))
mag = np.abs(X)
check(np.allclose(mag[[5, n - 5]], n / 2) and np.allclose(np.delete(mag, [5, n - 5]), 0, atol=1e-9), "fft of a cosine: peaks n/2 at bins 5 and n-5")
check(np.allclose(fft.ifft(X).real, np.cos(2 * np.pi * 5 * t / n)), "ifft inverts fft")
q, err = integrate.quad(lambda v: math.exp(-v * v), -np.inf, np.inf)
check(abs(q - math.sqrt(math.pi)) < 1e-10, "integrate.quad: sqrt(pi)")
sol = integrate.solve_ivp(lambda tt, y: -y, (0, 2), [1.0], rtol=1e-10, atol=1e-12)
check(abs(sol.y[0, -1] - math.exp(-2)) < 1e-8, "integrate.solve_ivp: y' = -y gives exp(-2)")
check(special.gamma(5) == 24.0 and abs(special.erf(1.0) - 0.8427007929497149) < 1e-15, "special.gamma(5) = 24, erf(1)")
M = sparse.csr_matrix(np.array([[0, 0, 3], [4, 0, 0], [0, 5, 0]], dtype=float))
check(np.array_equal(M @ np.array([1.0, 2.0, 3.0]), [9.0, 4.0, 10.0]) and M.nnz == 3, "sparse CSR matrix-vector product")
check(np.allclose(sparse.linalg.spsolve(M.tocsc(), np.array([9.0, 4.0, 10.0])), [1.0, 2.0, 3.0]), "sparse.linalg.spsolve (SuperLU)")
check(abs(stats.norm.cdf(1.96) - 0.9750021048517795) < 1e-12, "stats.norm.cdf(1.96)")
f = interpolate.CubicSpline([0, 1, 2, 3], [0, 1, 8, 27])
check(abs(float(f(1.5)) - 3.375) < 0.1, "interpolate.CubicSpline of x^3")
import scipy.optimize._group_columns as gc
check(gc.__file__.endswith(".so"), "a Pythran-built module loads (optimize._group_columns)")
try:
    import scipy.odr
    info("scipy.odr present")
except ImportError:
    info("scipy.odr absent (Fortran; not built)")
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-scipy"; exit 0; }
echo "FAIL python3-scipy (rc=$rc)"; exit 1
