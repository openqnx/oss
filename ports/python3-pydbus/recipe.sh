# pydbus: a Pythonic D-Bus library on GDBus (PyGObject's Gio).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.6.0
TARBALL=https://files.pythonhosted.org/packages/92/56/27148014c2f85ce70332f18612f921f682395c7d4e91ec103783be4fce00/pydbus-0.6.0-py2.py3-none-any.whl
SHA256=66b80106352a718d80d6c681dc2a82588048e30b75aab933e4020eb0660bf85e
DEPS="python3 python3-gobject3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-gobject3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
