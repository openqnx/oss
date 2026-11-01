# colcon-recursive-crawl: extension for colcon to recursively crawl for packages.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.2.3
TARBALL=https://files.pythonhosted.org/packages/89/1c/04c4382add856e6109cc9a2beca3f40aeceabce4d09cf832ea1bac97c3d9/colcon_recursive_crawl-0.2.3-py3-none-any.whl
SHA256=67b5f35d702728c0b638bfa581ca54699871dea7817f4415a29b782ee5b5e135
DEPS="python3 python3-colcon-core"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
