# flake8-import-order: flake8 plugin checking the order of imports (I1xx).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.19.2
TARBALL=https://files.pythonhosted.org/packages/18/d4/3d067f1c4a429e82ec9ae54a346ef50e4d317c6cdfba6bd1443c162ff39f/flake8_import_order-0.19.2-py3-none-any.whl
SHA256=2dfe60175e7195cf36d4c573861fd2e3258cd6650cbd7616da3c6b8193b29b7c
DEPS="python3 python3-pycodestyle python3-setuptools python3-flake8"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-pycodestyle python3-setuptools python3-flake8"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
