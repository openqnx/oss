# colcon-cmake: extension for colcon to support CMake packages.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.2.30
TARBALL=https://files.pythonhosted.org/packages/00/65/71f1b811a60c6f116729c2ae4806c2bc47fa17bb5ac697db39f5439121de/colcon_cmake-0.2.30-py3-none-any.whl
SHA256=2be9af465f64babd588e296f61c777d2c8386f9a82477dd3da7cf5142db6367d
DEPS="python3 python3-colcon-core python3-colcon-library-path python3-colcon-test-result python3-packaging"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core python3-colcon-library-path python3-colcon-test-result python3-packaging"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
