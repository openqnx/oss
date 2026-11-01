# pytest-rerunfailures: pytest plugin to re-run tests to eliminate flaky failures.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=16.7
TARBALL=https://files.pythonhosted.org/packages/c1/d3/07ea35102cf2020ddaaac368d2a6f0bcc63d6523fb8918805bac3eb8b98f/pytest_rerunfailures-16.7-py3-none-any.whl
SHA256=edf1886209c2b7dafe35b5bf1708d6ec40ccf6c6b357f0f02807efcec0204c99
DEPS="python3 python3-packaging python3-pytest"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-packaging python3-pytest"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
