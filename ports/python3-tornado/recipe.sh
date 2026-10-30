# tornado: web framework and asynchronous networking (HTTP server and client, IOLoop, websockets).
# Pure Python, published as a source distribution only: built into a
# platform-independent wheel here (see scripts/lib/pywheel.sh).
VERSION=6.6
TARBALL=https://files.pythonhosted.org/packages/d0/02/5f549b1ad5da6267ed1e1fac2eef69257b1979cc812497002499b7895ff9/tornado-6.6.tar.gz
SHA256=34c4aa916d7f2b2d5cadee2c6850f5a4a7f3d688cbf58399ea9bd3057af4abd5
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
# Its C speed-ups (tornado.speedups) are optional: the pure-Python ones are used.
do_build() { TORNADO_EXTENSION=0 qnx_py_wheel_pure; }
do_install() { qnx_py_install; }
