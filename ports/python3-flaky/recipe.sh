# flaky: a pytest plugin that reruns flaky tests.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.8.1
TARBALL=https://files.pythonhosted.org/packages/7f/b8/b830fc43663246c3f3dd1ae7dca4847b96ed992537e85311e27fa41ac40e/flaky-3.8.1-py2.py3-none-any.whl
SHA256=194ccf4f0d3a22b2de7130f4b62e45e977ac1b5ccad74d4d48f3005dcc38815e
DEPS="python3 python3-pytest"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-pytest"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
