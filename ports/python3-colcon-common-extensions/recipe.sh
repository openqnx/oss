# colcon-common-extensions: meta package aggregating colcon-core and common extensions.
# The meta package: it installs no code, only depends on colcon-core and the
# common extensions (DEPS).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.3.0
TARBALL=https://files.pythonhosted.org/packages/82/a4/04d5840a3ca8e13fc95dc710f9b8c97b379573505291ba42ffa92c94e959/colcon_common_extensions-0.3.0-py3-none-any.whl
SHA256=a2acb90de17c82dc7e4ebd3c04d73833d7c12670c3db71b50db8fcd84d560694
DEPS="python3 python3-colcon-argcomplete python3-colcon-bash python3-colcon-cd python3-colcon-cmake python3-colcon-core python3-colcon-defaults python3-colcon-devtools python3-colcon-library-path python3-colcon-metadata python3-colcon-notification python3-colcon-output python3-colcon-package-information python3-colcon-package-selection python3-colcon-parallel-executor python3-colcon-powershell python3-colcon-python-setup-py python3-colcon-recursive-crawl python3-colcon-ros python3-colcon-test-result python3-colcon-zsh"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-argcomplete python3-colcon-bash python3-colcon-cd python3-colcon-cmake python3-colcon-core python3-colcon-defaults python3-colcon-devtools python3-colcon-library-path python3-colcon-metadata python3-colcon-notification python3-colcon-output python3-colcon-package-information python3-colcon-package-selection python3-colcon-parallel-executor python3-colcon-powershell python3-colcon-python-setup-py python3-colcon-recursive-crawl python3-colcon-ros python3-colcon-test-result python3-colcon-zsh"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
