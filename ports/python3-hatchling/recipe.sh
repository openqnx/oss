# hatchling: the build backend of Hatch.
# Version 1.31.0: the last release before tomlkit became a dependency.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers (qnx_py_scripts).
VERSION=1.33.0
TARBALL=https://files.pythonhosted.org/packages/32/64/4afe5dd77b2939a93e8c9513f8082dbd049ecba9c1484a379a0f6ae29d6b/hatchling-1.33.0-py3-none-any.whl
SHA256=415290b84f464d2884303fca8974d38e7b85579069fab02a15db0828fbb1446d
DEPS="python3 python3-packaging python3-pathspec python3-pluggy python3-trove-classifiers"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-packaging python3-pathspec python3-pluggy python3-trove-classifiers"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
