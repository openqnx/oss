# colcon-test-result: extension for colcon to provide information about test results.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.3.8
TARBALL=https://files.pythonhosted.org/packages/b6/62/a3e327473df5557fc0a2f2d3bc6327b56cbf3df4a2626ab6f58b1da9dcec/colcon_test_result-0.3.8-py3-none-any.whl
SHA256=0c54f701ffd4593c515b9bf2db16adf5c26526ed81ab6b4e4e9d867ed690e782
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
