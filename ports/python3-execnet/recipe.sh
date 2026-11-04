# execnet: distributed Python execution over channels (pytest-xdist's workers).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=2.1.2
TARBALL=https://files.pythonhosted.org/packages/ab/84/02fc1827e8cdded4aa65baef11296a9bbe595c474f0d6d758af082d849fd/execnet-2.1.2-py3-none-any.whl
SHA256=67fba928dd5a544b783f6056f449e5e3931a5c378b128bc18501f7ea79e296ec
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
