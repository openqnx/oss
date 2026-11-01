# kiwisolver: the Cassowary constraint solver (C++) for Python, used by
# matplotlib's constrained layout. A setuptools C++ extension (with cppy's
# helpers), cross compiled like numpy (scripts/lib/pywheel.sh); the source
# distribution carries its version, which setuptools_scm would otherwise
# take from git.
VERSION=1.5.1
TARBALL=https://files.pythonhosted.org/packages/ba/07/bd78e6a8fae171ea041ef5bba3ed21a003522fa088834b069b1909981f30/kiwisolver-1.5.1.tar.gz
SHA256=f1303ef2eec81262a4b708c3e858afe58d7c75ad91c1c05266eda7673369859a
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=61.2" "setuptools_scm[toml]>=3.4.3" "cppy>=1.3.0"; }
# The target's Python headers first (the venv's are the build machine's);
# the C++ module is linked by the C++ driver, so that libc++ is linked.
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  CXXFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  LDCXXSHARED="q++ -V${QCC_TARGET}_cxx -shared" qnx_py_wheel
}
do_install() { qnx_py_install; }
