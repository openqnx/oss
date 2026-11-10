# flake8-quotes: flake8 plugin for consistent string quotes (Q000-Q003).
# Pure Python, published only as a source distribution: the build
# machine's Python builds the platform independent wheel (scripts/lib/pywheel.sh).
VERSION=3.4.0
TARBALL=https://files.pythonhosted.org/packages/dd/57/a173e3eb86072b7ee77650aca496b15d6886367d257f58ea9de5276e330a/flake8-quotes-3.4.0.tar.gz
SHA256=aad8492fb710a2d3eabe68c5f86a1428de650c8484127e14c43d0504ba30276c
DEPS="python3 python3-flake8 python3-setuptools"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-flake8 python3-setuptools"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_build() { qnx_py_wheel_pure; }
do_install() { qnx_py_install; }
