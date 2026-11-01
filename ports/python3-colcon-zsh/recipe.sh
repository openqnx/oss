# colcon-zsh: extension for colcon to provide Z shell scripts.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.5.0
TARBALL=https://files.pythonhosted.org/packages/56/15/77a9f6d9745713ef117920268ec5e32dfcf16e9ba373a2299c0952d91e71/colcon_zsh-0.5.0-py3-none-any.whl
SHA256=a7697630d6763f4a9599d87e42ce9e7cf541873b8f40b0d77ed1eaf9510fd55a
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
