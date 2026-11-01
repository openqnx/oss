# colcon-cd: a shell function for colcon to change the current working directory.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.1.1
TARBALL=https://files.pythonhosted.org/packages/b1/83/58212820793dba91ac3d123cffae5eb3d51543116e8341ebf0dda87a5d5e/colcon_cd-0.1.1-py3-none-any.whl
SHA256=3ca8f3af5d5e9a956db23797e742cbe5819762f81f731e882aecded8689ecfe6
DEPS="python3 python3-colcon-core python3-colcon-package-information"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core python3-colcon-package-information"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
# The wheel's .data/ (share/ files, scripts, headers) goes where pip puts it.
source "$WS/scripts/lib/pywheel-data.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_install_data; }
