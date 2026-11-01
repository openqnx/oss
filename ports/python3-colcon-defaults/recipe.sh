# colcon-defaults: extension for colcon to read defaults from a config file.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.2.9
TARBALL=https://files.pythonhosted.org/packages/9b/8b/19dfa6fee7d5bb4d1894e27adb4811ef6c9c983a25afcf261fc04b43f6c1/colcon_defaults-0.2.9-py3-none-any.whl
SHA256=801ab5f6ff845a986425a9bad7e2131cf0b90523cc7ce276eaeea1b49d51efaa
DEPS="python3 python3-colcon-core python3-pyyaml"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core python3-pyyaml"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
