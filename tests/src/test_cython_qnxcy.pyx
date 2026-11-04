# cython: language_level=3
# The extension module test_cython carries: compiled at build time by the
# ported Cython (run on the build machine) and the QNX compiler, imported on
# the target. Typed C code, a cdef class, C math, exceptions, memoryviews.
from libc.math cimport sqrt
from libc.string cimport strlen

def fib(int n):
    """n-th Fibonacci number, computed in a typed C loop."""
    cdef long long a = 0, b = 1, t
    cdef int i
    if n < 0:
        raise ValueError("negative n")
    for i in range(n):
        t = a + b
        a = b
        b = t
    return a

def primes(int limit):
    """Primes below limit (sieve over a C array)."""
    cdef bytearray sieve = bytearray([1]) * limit
    cdef unsigned char[:] s = sieve
    cdef int i, j
    out = []
    for i in range(2, limit):
        if s[i]:
            out.append(i)
            j = i * i
            while j < limit:
                s[j] = 0
                j += i
    return out

def norm(double[:] v):
    """Euclidean norm of a buffer of doubles (libc sqrt)."""
    cdef double acc = 0
    cdef Py_ssize_t i
    for i in range(v.shape[0]):
        acc += v[i] * v[i]
    return sqrt(acc)

def clen(bytes s):
    return strlen(s)

cdef class Counter:
    cdef public int count
    cdef int step
    def __init__(self, int step=1):
        self.count = 0
        self.step = step
    cpdef int bump(self):
        self.count += self.step
        return self.count
