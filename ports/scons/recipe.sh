# SCons: a software construction tool (a make replacement) whose build
# files are Python.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=4.11.1
TARBALL=https://files.pythonhosted.org/packages/8e/43/d6285848e893c19682c06e92679dc1a07d37ff7ea148747b1df681ec496c/scons-4.11.1-py3-none-any.whl
SHA256=454cef364348053422696e3d2ecb4fa593c96a624f955842eaaea64f95c8d11d
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
