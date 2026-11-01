# colcon-powershell: extension for colcon to provide PowerShell scripts.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.5.0
TARBALL=https://files.pythonhosted.org/packages/3e/85/2c191f8682f4b4ad2f7cc6374fca8b5bab6c75495991fa2baca08f7ae6df/colcon_powershell-0.5.0-py3-none-any.whl
SHA256=1382fba58c451d79ac42d1a429762406f4c3f695a53a2d0254570c93406e0793
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
