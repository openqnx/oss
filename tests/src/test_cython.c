/* Cython: an extension module (test_cython_qnxcy.pyx) compiled at build time
 * -- the ported Cython generates the C on the build machine, the QNX
 * compiler builds it against the ported python3's headers -- is carried in
 * this program; the test writes it out as qnxcy.so and has the target's
 * python3 import it: typed loops (fib, a sieve), libc math and strlen,
 * memoryviews, a cdef class with a cpdef method, exceptions raised in C.
 * Then Cython itself on the target: its compiled modules (Scanning, Parsing,
 * Code, refnanny) load as extension modules, the cython command reports its
 * version and compiles a .pyx to C (no C compiler needed for that step).
 * Needs python3 and cython installed (QNX_PORT_PYTHON overrides python3). */
#include "common.h"
#include "qnxcy_so.h"   /* qnxcy_so[], qnxcy_so_len: the module */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static const char *script =
  "import sys, array\n"
  "fails = 0\n"
  "def check(ok, what):\n"
  "    global fails\n"
  "    print(('  ok   ' if ok else '  FAIL ') + what); fails += (not ok)\n"
  "import qnxcy as m\n"
  "print('  info module', m.__file__)\n"
  "check(m.fib(0) == 0 and m.fib(10) == 55 and m.fib(90) == 2880067194370816120, 'fib(): typed C loop, 64-bit result')\n"
  "check(m.primes(30) == [2, 3, 5, 7, 11, 13, 17, 19, 23, 29] and len(m.primes(10000)) == 1229, 'primes(): sieve over a memoryview')\n"
  "check(m.norm(array.array('d', [3.0, 4.0, 12.0])) == 13.0, 'norm(): double[:] buffer, libc sqrt')\n"
  "check(m.clen(b'neutrino') == 8, 'clen(): libc strlen on bytes')\n"
  "try:\n"
  "    m.fib(-1); check(False, 'ValueError from C code')\n"
  "except ValueError as e:\n"
  "    check(str(e) == 'negative n', 'ValueError raised in C code, with its message')\n"
  "try:\n"
  "    m.fib('x'); check(False, 'TypeError for a non-int argument')\n"
  "except TypeError:\n"
  "    check(True, 'TypeError for a non-int argument')\n"
  "c = m.Counter(3); c.bump(); c.bump()\n"
  "check(c.count == 6 and isinstance(c, m.Counter), 'cdef class: public attribute, cpdef method')\n"
  "import Cython, Cython.Compiler.Scanning as sc, Cython.Compiler.Parsing as pa, Cython.Compiler.Code as co\n"
  "import Cython.Runtime.refnanny as rn\n"
  "print('  info Cython', Cython.__version__)\n"
  "check(all(x.__file__.endswith('.so') for x in (sc, pa, co, rn)), 'Cython compiler modules and refnanny are compiled extensions')\n"
  "sys.exit(1 if fails else 0)\n";

static const char *tiny = "def twice(int x):\n    return 2 * x\n";

int main(void) {
  char dir[256], path[300], cmd[1400], out[512] = ""; FILE *f; int rc;
  const char *py = getenv("QNX_PORT_PYTHON");
  const char *cy = access("/usr/local/bin/cython", X_OK) == 0 ? "/usr/local/bin/cython" : "cython";
  QT_BEGIN("cython");
  /* no __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183) */
  setenv("PYTHONDONTWRITEBYTECODE", "1", 1);
  if (!py) py = access("/usr/local/bin/python3", X_OK) == 0 ? "/usr/local/bin/python3" : "python3";
  snprintf(dir, sizeof dir, "%s/qnx-cython-%d", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp", (int)getpid());
  QT_CHECK(mkdir(dir, 0700) == 0, "temporary directory");
  snprintf(path, sizeof path, "%s/qnxcy.so", dir);
  f = fopen(path, "wb");
  QT_CHECK(f && fwrite(qnxcy_so, 1, qnxcy_so_len, f) == qnxcy_so_len && fclose(f) == 0, "module written as qnxcy.so");
  snprintf(path, sizeof path, "%s/t.py", dir);
  f = fopen(path, "w"); if (f) { fputs(script, f); fclose(f); }
  printf("  info module %u bytes, interpreter %s\n", (unsigned)qnxcy_so_len, py);
  snprintf(cmd, sizeof cmd, "cd '%s' && PYTHONPATH='%s' '%s' t.py", dir, dir, py);
  rc = system(cmd);
  QT_CHECK(rc == 0, "python3 imports the Cython-compiled module and every check in it passes");

  snprintf(cmd, sizeof cmd, "'%s' --version 2>&1", cy);
  f = popen(cmd, "r");
  if (f) { if (!fgets(out, sizeof out, f)) out[0] = 0; pclose(f); }
  printf("  info %s", out[0] ? out : "(no output)\n");
  QT_CHECK(strstr(out, "Cython version 3.") != NULL, "cython --version reports Cython 3.x");

  snprintf(path, sizeof path, "%s/tiny.pyx", dir);
  f = fopen(path, "w"); if (f) { fputs(tiny, f); fclose(f); }
  snprintf(cmd, sizeof cmd, "cd '%s' && '%s' -3 tiny.pyx -o tiny.c && grep -c 'PyInit_tiny' tiny.c >/dev/null", dir, cy);
  QT_CHECK(system(cmd) == 0, "cython compiles tiny.pyx to C on the target (tiny.c defines PyInit_tiny)");
  snprintf(cmd, sizeof cmd, "rm -rf '%s'", dir); system(cmd);
  QT_END();
}
