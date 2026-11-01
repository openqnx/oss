# colcon-bash: extension for colcon to provide Bash scripts.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.5.0
TARBALL=https://files.pythonhosted.org/packages/6e/f6/d48f6c8602ab80595e715c37172418524e6ef4ca8aa8ab66f01e7bf23fce/colcon_bash-0.5.0-py3-none-any.whl
SHA256=1c13dc3634c1cac1349afaaa4dabbc68d122bdb4d671da5f8c8c62bc05b8390a
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
