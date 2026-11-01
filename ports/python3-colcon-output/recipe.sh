# colcon-output: extension for colcon to customize the output in various ways.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.2.15
TARBALL=https://files.pythonhosted.org/packages/e5/0c/95b6551e797111e481994e04bcc77d225753f9ea6d2ffaa2c96fa9e6bcd8/colcon_output-0.2.15-py3-none-any.whl
SHA256=89649cd67f5c7991145730a70bee703d63b7883ff903831502023e9ccda3cce0
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
