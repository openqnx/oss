# psutil: process and system utilities for Python, with the QNX platform
# module from github.com/qnx-ports/psutil (patches/0001: _psutil_qnx and
# _psqnx.py -- CPU count and frequency, memory from /proc/vm/stats, network
# interfaces, processes through /proc/<pid>/as) plus fixes to it (0002-).
VERSION=7.2.2; GIT=https://github.com/giampaolo/psutil.git; REF=release-7.2.2
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_configure() { qnx_py_venv "setuptools>=43"; }
# A setuptools C extension, cross compiled: setup.py picks the platform
# module from sys.platform, which is the build machine's -- patches/0002
# takes it from _PYTHON_HOST_PLATFORM (set by qnx_py_wheel) instead. The
# target's Python headers come first on the include path (the venv's own
# are the build machine's, whose pyconfig.h is Linux's).
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
do_install() { qnx_py_install; }
