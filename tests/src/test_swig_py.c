/* swig, Python target language: test_swig_example.i was turned into py_wrap.c
 * and qnxswig.py at build time by swig -python (SWIG 4.3.1), py_wrap.c built
 * into the extension _qnxswig.so against the ported python3's headers. This
 * program carries both; it writes them to a temporary directory and has the
 * target's python3 import qnxswig and check functions, the global variable
 * (cvar), the struct proxy class, a returned new string, type errors.
 * "test_swig_py <dir>" only writes the two files to <dir> (test_swig uses
 * them). Needs python3 installed (QNX_PORT_PYTHON overrides it). */
#include "common.h"
#include "qnxswig_so.h"   /* qnxswig_so[]: _qnxswig.so */
#include "qnxswig_py.h"   /* qnxswig_py[]: qnxswig.py */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

static const char *script =
  "import sys, math\n"
  "fails = 0\n"
  "def check(ok, what):\n"
  "    global fails\n"
  "    print(('  ok   ' if ok else '  FAIL ') + what); fails += (not ok)\n"
  "import qnxswig as m\n"
  "check(m.gcd(84, 36) == 12 and m.gcd(-17, 5) == 1, 'gcd(84,36)=12, gcd(-17,5)=1 (qs_ prefix renamed away)')\n"
  "check(math.isclose(m.hypot3(2, 3, 6), 7.0), 'hypot3(2,3,6) = 7.0')\n"
  "check(m.crc8('123456789') == 0xF4, 'crc8(\"123456789\") = 0xF4')\n"
  "check(m.reverse('QNX swig') == 'giws XNQ', 'reverse(): %newobject string')\n"
  "a = m.acc()\n"
  "m.acc_add(a, 5); m.acc_add(a, 7)\n"
  "check(a.n == 2 and a.total == 12, 'struct proxy class acc: n=2 total=12')\n"
  "a.total = 100\n"
  "check(a.total == 100, 'struct member set from Python')\n"
  "check(m.cvar.calls == 7, 'global variable through cvar: 7 calls')\n"
  "try:\n"
  "    m.gcd('x', 1); check(False, 'wrong argument type -> TypeError')\n"
  "except TypeError:\n"
  "    check(True, 'wrong argument type -> TypeError')\n"
  "check(m._qnxswig.__file__.endswith('_qnxswig.so'), 'extension module _qnxswig.so loaded')\n"
  "sys.exit(1 if fails else 0)\n";

static int put(const char *dir, const char *name, const unsigned char *d, size_t n) {
  char path[400]; FILE *f;
  snprintf(path, sizeof path, "%s/%s", dir, name);
  f = fopen(path, "wb");
  return f && fwrite(d, 1, n, f) == n && fclose(f) == 0;
}

int main(int argc, char **argv) {
  char dir[256], cmd[1024]; int rc;
  const char *py = getenv("QNX_PORT_PYTHON");
  if (argc > 1)
    return put(argv[1], "_qnxswig.so", qnxswig_so, qnxswig_so_len) && put(argv[1], "qnxswig.py", qnxswig_py, qnxswig_py_len) ? 0 : 1;
  QT_BEGIN("swig_py");
  if (!py) py = access("/usr/local/bin/python3", X_OK) == 0 ? "/usr/local/bin/python3" : "python3";
  snprintf(dir, sizeof dir, "%s/qnx-swig-%d", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp", (int)getpid());
  QT_CHECK(mkdir(dir, 0700) == 0, "temporary directory");
  QT_CHECK(put(dir, "_qnxswig.so", qnxswig_so, qnxswig_so_len), "extension written as _qnxswig.so");
  QT_CHECK(put(dir, "qnxswig.py", qnxswig_py, qnxswig_py_len), "shadow module written as qnxswig.py");
  QT_CHECK(put(dir, "t.py", (const unsigned char *)script, strlen(script)), "check script written");
  printf("  info extension %u bytes, interpreter %s\n", (unsigned)qnxswig_so_len, py);
  snprintf(cmd, sizeof cmd, "cd '%s' && PYTHONPATH='%s' '%s' t.py", dir, dir, py);
  rc = system(cmd);
  QT_CHECK(rc == 0, "python3 imports qnxswig and every check in it passes");
  snprintf(cmd, sizeof cmd, "rm -rf '%s'", dir); system(cmd);
  QT_END();
}
