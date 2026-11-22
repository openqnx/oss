# tornado: web framework and asynchronous networking (HTTP server and client, IOLoop, websockets).
# Pure Python, published as a source distribution only: built into a
# platform-independent wheel here (see scripts/lib/pywheel.sh).
VERSION=6.6.1
TARBALL=https://files.pythonhosted.org/packages/64/2b/8ff2b41620cbba0eeb0862fed21dcaa5a2ad36c6dd9bb465f9ff7a147be2/tornado-6.6.1.tar.gz
SHA256=28e6311402af59c8b02e3fa0f5e7d8eb465b25081381803b127d97e6a4395d5a
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
# Its C speed-ups (tornado.speedups) are optional: the pure-Python ones are used.
do_build() { TORNADO_EXTENSION=0 qnx_py_wheel_pure; }
do_install() { qnx_py_install; }
