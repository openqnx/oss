# wheel (version as in the QNX Everywhere 8.0.4r1 image).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.46.1
TARBALL=https://files.pythonhosted.org/packages/af/44/31d964a260fcda6bf2f59384231bbf4962ac796a5d36c554162b5411b8db/wheel-0.46.1-py3-none-any.whl
SHA256=f796f65d72750ccde090663e466d0ca37cd72b62870f7520b96d34cdc07d86d8
DEPS="python3 python3-packaging"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
