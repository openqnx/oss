# colcon-parallel-executor: extension for colcon to process packages in parallel.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.4.0
TARBALL=https://files.pythonhosted.org/packages/c7/4b/60377305e7671736df4abd9130089a9d4726ee768677a35ce76b6ed44fb4/colcon_parallel_executor-0.4.0-py3-none-any.whl
SHA256=d3fe7ba1c8db2fb27d6be077605be6520a711940cc1601a45239ee941987a9e9
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
