/* CPython embedded: initialise the interpreter, run code, call into extension
 * modules that depend on other ports (zlib, ssl, ctypes/libffi, sqlite3). */
#include "common.h"
#include <stdlib.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
static int run(const char *code) { return PyRun_SimpleString(code) == 0; }
int main(void) {
  QT_BEGIN("python3");
  printf("  info %s\n", Py_GetVersion());
  /* Pin the interpreter to the stdlib shipped with these tests.  Left to
     itself, CPython searches PATH for "python3" to find its prefix, and a
     target image that ships QNX's own Python hands it /system -- whose stdlib
     and lib-dynload modules are prebuilt SDP binaries.  That is how this test
     once passed on the Pi without touching our _ssl, _ctypes or _sqlite3. */
  const char *home = getenv("QNX_PORT_PYTHONHOME");
  QT_CHECK(home != NULL, "QNX_PORT_PYTHONHOME names the bundled stdlib");
  if (!home) QT_END();
  PyConfig config;
  PyConfig_InitIsolatedConfig(&config);
  config.write_bytecode = 0;
  PyStatus st = PyConfig_SetBytesString(&config, &config.home, home);
  if (!PyStatus_Exception(st))
    st = Py_InitializeFromConfig(&config);
  PyConfig_Clear(&config);
  QT_CHECK(!PyStatus_Exception(st), "Py_InitializeFromConfig");
  if (PyStatus_Exception(st)) {
    printf("  info %s: %s\n", st.func ? st.func : "?", st.err_msg ? st.err_msg : "?");
    QT_END();
  }
  QT_CHECK(Py_IsInitialized(), "Py_IsInitialized");

  /* Every module below must come from the bundle, never from the system. */
  PyObject *h = PyUnicode_DecodeFSDefault(home);
  PyObject *main_dict = PyModule_GetDict(PyImport_AddModule("__main__"));
  PyDict_SetItemString(main_dict, "HOME", h);
  Py_XDECREF(h);
  QT_CHECK(run("import os, sys\n"
               "def ours(m):\n"
               "    f = os.path.realpath(m.__file__)\n"
               "    assert f.startswith(os.path.realpath(HOME) + '/'), f'{m.__name__} from {f}'\n"
               "import _ssl, _ctypes, _sqlite3, zlib, _bz2, _lzma, pyexpat\n"
               "for m in (os, _ssl, _ctypes, _sqlite3, zlib, _bz2, _lzma, pyexpat): ours(m)\n"
               "print('  info stdlib:', os.path.dirname(os.__file__))"),
           "stdlib and extension modules all load from the bundle");

  QT_CHECK(run("import sys; assert sys.version_info[:2] == (3, 14)"), "version is 3.14");
  QT_CHECK(run("assert sum(i*i for i in range(10)) == 285"), "run a generator expression");
  QT_CHECK(run("import platform; print('  info platform:', platform.system(), platform.machine())"),
           "platform module");

  /* Extension modules, each one a port in this workspace. */
  QT_CHECK(run("import zlib; assert zlib.decompress(zlib.compress(b'qnx'*100)) == b'qnx'*100"),
           "zlib module round trip");
  QT_CHECK(run("import bz2; assert bz2.decompress(bz2.compress(b'qnx')) == b'qnx'"),
           "bz2 module");
  QT_CHECK(run("import lzma; assert lzma.decompress(lzma.compress(b'qnx')) == b'qnx'"),
           "lzma module");
  QT_CHECK(run("import binascii, hashlib;"
               "assert hashlib.sha256(b'abc').hexdigest() =="
               " 'ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'"),
           "hashlib SHA-256 vector");
  QT_CHECK(run("import ssl; print('  info ssl:', ssl.OPENSSL_VERSION)"), "ssl module (OpenSSL)");
  QT_CHECK(run("import sqlite3;"
               "c = sqlite3.connect(':memory:');"
               "c.execute('create table t(a int)');"
               "c.execute('insert into t values (42)');"
               "assert c.execute('select a from t').fetchone()[0] == 42"),
           "sqlite3 module");
  QT_CHECK(run("import ctypes, ctypes.util;"
               "libc = ctypes.CDLL(None);"
               "libc.abs.restype = ctypes.c_int;"
               "assert libc.abs(-7) == 7"),
           "ctypes calling through libffi");
  QT_CHECK(run("import ctypes;"
               "CB = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_int);"
               "cb = CB(lambda x: x * 2);"
               "assert cb(21) == 42"),
           "a ctypes callback (libffi closure -- executable memory)");
  QT_CHECK(run("import _socket, select, os, threading, time"), "core extension modules import");
  QT_CHECK(run("import xml.etree.ElementTree as ET;"
               "assert ET.fromstring('<a><b/></a>')[0].tag == 'b'"),
           "expat-backed XML parser");
  QT_CHECK(run("import unicodedata; assert unicodedata.name('A') == 'LATIN CAPITAL LETTER A'"),
           "unicodedata");
  QT_CHECK(run("import decimal; assert str(decimal.Decimal(1)/decimal.Decimal(7))[:10] == '0.14285714'"),
           "decimal (libmpdec)");
  /* Threading exercises QNX's pthread implementation through CPython's GIL. */
  QT_CHECK(run("import threading;"
               "r = [];"
               "ts = [threading.Thread(target=lambda i=i: r.append(i*i)) for i in range(8)];"
               "[t.start() for t in ts]; [t.join() for t in ts];"
               "assert sorted(r) == [i*i for i in range(8)]"),
           "threads run and join");
  QT_CHECK(Py_FinalizeEx() == 0, "Py_FinalizeEx");
  QT_END();
}
