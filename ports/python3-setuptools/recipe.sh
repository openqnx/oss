# setuptools (version as in the QNX Everywhere 8.0.4r1 image).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=80.8.0
TARBALL=https://files.pythonhosted.org/packages/58/29/93c53c098d301132196c3238c312825324740851d77a8500a2462c0fd888/setuptools-80.8.0-py3-none-any.whl
SHA256=95a60484590d24103af13b686121328cc2736bee85de8936383111e421b9edc0
DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
