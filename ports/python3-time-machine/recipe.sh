# time-machine: travel through time in Python tests -- mocks the clock
# (time.time, datetime.now, ...) by patching the C functions behind them. A
# setuptools C extension (_time_machine), cross compiled like netifaces
# (scripts/lib/pywheel.sh).
VERSION=3.6.0
TARBALL=https://files.pythonhosted.org/packages/a7/1d/48df18d3b2159ae3f49432622e3d7a23e01404cd6b99ee1361c9a4f37826/time_machine-3.6.0.tar.gz
SHA256=92dfc900bbd50d239ad311473ada2201acdca3c3e14562692fd2e31489b57e24
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
do_install() { qnx_py_install; }
