# pytest-mock: the mocker fixture for pytest.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.17.0
TARBALL=https://files.pythonhosted.org/packages/e8/ea/47fbdc8f37410956fd445720e927589b1e2a47772c5f28109f0ce2fa26d9/pytest_mock-3.17.0-py3-none-any.whl
SHA256=1b82aa4b37137248bd687055e109723debeb07b489de1565297dc9589c5f2d6b
DEPS="python3 python3-pytest"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-pytest"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
