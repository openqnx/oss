# colcon-library-path: extension for colcon adding an environment variable to find libraries.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.2.1
TARBALL=https://files.pythonhosted.org/packages/b8/02/4e4dd0cb94491774e87cf479066a8b6ef1f653889a0c9fb957f527c165f7/colcon_library_path-0.2.1-py3-none-any.whl
SHA256=e8dd2f87bf2a8776b4770004dd744d16858cb6338eb170fe934066236f4eab21
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
