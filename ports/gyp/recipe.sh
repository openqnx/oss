# GYP (gyp-next, the maintained fork used by Node.js): "Generate Your
# Projects", turns .gyp files into Makefiles or Ninja files.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=0.22.4
TARBALL=https://files.pythonhosted.org/packages/dc/d9/bcd41b8aae1bb78735c13438b1a2e10b1b1129f28ec31da6f808d3fd3fc4/gyp_next-0.22.4-py3-none-any.whl
SHA256=303f2f8f6db31465562eeb519d10eceaf44eef55a58ed304e76e44f5b1433d9a
DEPS="python3 python3-packaging python3-setuptools"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-packaging python3-setuptools"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
