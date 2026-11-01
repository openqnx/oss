# colcon-package-selection: extension for colcon to select the packages to process.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.2.10
TARBALL=https://files.pythonhosted.org/packages/6f/58/fb91adaab07ed20865839acd45a6f88a030751c19688e4781d556d34a1b5/colcon_package_selection-0.2.10-py3-none-any.whl
SHA256=a918f1997121e6b3410cf7944844624ce8a7abff676788a8b9128629e012989e
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
