# colcon-devtools: extension for colcon listing its extension points and extensions.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.3.0
TARBALL=https://files.pythonhosted.org/packages/61/5e/564a69cb701bf2d8ef3d4ee9971f901d021f7f283f28f8d0e4d17af4c94d/colcon_devtools-0.3.0-py3-none-any.whl
SHA256=25528e32cdcaed5ede43c99fd502d7d970ab22c5ebc9b45613f95bd855c18585
DEPS="python3 python3-colcon-core python3-packaging"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core python3-packaging"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
