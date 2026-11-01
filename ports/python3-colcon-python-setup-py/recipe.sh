# colcon-python-setup-py: extension for colcon to support setup.py Python packages.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.2.9
TARBALL=https://files.pythonhosted.org/packages/9a/74/df3a38ab9c2f9f12bac04eed4dbdeedf29d04c3537d23cd83aa3e85973bd/colcon_python_setup_py-0.2.9-py3-none-any.whl
SHA256=6ba76adb954cb963ada852dd01fa33890ba71638c48cdb43404acb0b52e98163
DEPS="python3 python3-colcon-core python3-setuptools"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-colcon-core python3-setuptools"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
