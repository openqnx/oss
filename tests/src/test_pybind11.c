/* pybind11: a small extension module (test_pybind11_mod.cpp, built against
 * pybind11 and the ported python3's headers, not linked to libpython) is
 * carried in this program; the test writes it to a temporary directory as
 * qnxpb11.so and has the target's python3 import it and check its
 * functions, class, STL conversions, exception translation and callbacks.
 * Needs python3 installed (QNX_PORT_PYTHON overrides the interpreter). */
#include "common.h"
#include "qnxpb11_so.h"   /* qnxpb11_so[], qnxpb11_so_len: the module */
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

static const char *script =
  "import sys\n"
  "fails = 0\n"
  "def check(ok, what):\n"
  "    global fails\n"
  "    print(('  ok   ' if ok else '  FAIL ') + what); fails += (not ok)\n"
  "import qnxpb11 as m\n"
  "print('  info module', m.__file__)\n"
  "check(m.__doc__ == 'pybind11 port test module', 'module docstring')\n"
  "check(m.add(2, 3) == 5 and m.add(5) == 15 and m.add(b=1, a=2) == 3, 'add(): positional, default, keyword arguments')\n"
  "check(abs(m.total([0.5, 1.25, 2.25]) - 4.0) < 1e-12, 'total(list) via std::vector<double>')\n"
  "check(m.lengths(['qnx', 'neutrino']) == {'qnx': 3, 'neutrino': 8}, 'lengths(): std::vector<std::string> -> std::map -> dict')\n"
  "try:\n"
  "    m.fail('bad input'); check(False, 'std::invalid_argument -> ValueError')\n"
  "except ValueError as e:\n"
  "    check(str(e) == 'bad input', 'std::invalid_argument -> ValueError with its message')\n"
  "try:\n"
  "    m.add('x'); check(False, 'wrong argument type -> TypeError')\n"
  "except TypeError:\n"
  "    check(True, 'wrong argument type -> TypeError')\n"
  "check(m.apply_twice(lambda x: x * 3, 2) == 18, 'Python callable called from C++ (std::function)')\n"
  "p = m.Pet('Rex', 3)\n"
  "p.age = 4; p.name = 'Max'\n"
  "check(p.greet() == 'Max is 4' and repr(p) == '<Pet Max>', 'class Pet: constructor, properties, method, __repr__')\n"
  "try:\n"
  "    p.age = -1; check(False, 'property setter raises ValueError')\n"
  "except ValueError:\n"
  "    check(p.age == 4, 'property setter raises ValueError, value kept')\n"
  "check(isinstance(p, m.Pet) and m.Pet.__name__ == 'Pet', 'isinstance and type name')\n"
  "sys.exit(1 if fails else 0)\n";

int main(void) {
  char dir[256], path[300], cmd[1024]; FILE *f; int rc;
  const char *py = getenv("QNX_PORT_PYTHON");
  QT_BEGIN("pybind11");
  if (!py) py = access("/usr/local/bin/python3", X_OK) == 0 ? "/usr/local/bin/python3" : "python3";
  snprintf(dir, sizeof dir, "%s/qnx-pybind11-%d", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp", (int)getpid());
  QT_CHECK(mkdir(dir, 0700) == 0, "temporary directory");
  snprintf(path, sizeof path, "%s/qnxpb11.so", dir);
  f = fopen(path, "wb");
  QT_CHECK(f && fwrite(qnxpb11_so, 1, qnxpb11_so_len, f) == qnxpb11_so_len && fclose(f) == 0, "module written as qnxpb11.so");
  snprintf(path, sizeof path, "%s/t.py", dir);
  f = fopen(path, "w"); if (f) { fputs(script, f); fclose(f); }
  printf("  info module %u bytes, interpreter %s\n", (unsigned)qnxpb11_so_len, py);
  snprintf(cmd, sizeof cmd, "cd '%s' && PYTHONPATH='%s' '%s' t.py", dir, dir, py);
  rc = system(cmd);
  QT_CHECK(rc == 0, "python3 imports the module and every check in it passes");
  snprintf(cmd, sizeof cmd, "rm -rf '%s'", dir); system(cmd);
  QT_END();
}
