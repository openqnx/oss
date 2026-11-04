# gpep517: a minimal PEP 517 backend runner for distribution packaging.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers (qnx_py_scripts).
VERSION=23
TARBALL=https://files.pythonhosted.org/packages/cd/88/88272b009fe39fbd329432ba6038cf95cfb5def93a457c7216d6d174496c/gpep517-23-py3-none-any.whl
SHA256=132b89288b6bc28d8be872dcff6c68d61d57bd96e5ca7f9eaf2384bf33c52755
DEPS="python3 python3-installer"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-installer"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
