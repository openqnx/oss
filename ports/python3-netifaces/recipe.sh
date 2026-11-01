# netifaces: network interface addresses (netifaces.interfaces(),
# ifaddresses(), gateways()) for Python. A setuptools C extension, cross
# compiled like Pillow (scripts/lib/pywheel.sh). Its setup.py probes the
# platform by compiling and linking small test programs (getifaddrs,
# getnameinfo, sa_len, routing sockets): QNX keeps all of these in
# libsocket (patches/0001).
VERSION=0.11.0
TARBALL=https://files.pythonhosted.org/packages/a6/91/86a6eac449ddfae239e93ffc1918cf33fd9bab35c04d1e963b311e347a73/netifaces-0.11.0.tar.gz
SHA256=043a79146eb2907edf439899f262b3dfe41717d34124298ed281139a8b93ca32
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"   # setup.py caches its probe results in the build tree
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  qnx_py_wheel
}
do_install() { qnx_py_install; }
