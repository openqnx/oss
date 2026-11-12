# argparse-addons: additional argparse types and actions (cantools' command
# line uses them). Pure Python: the published wheel is installed as released
# (scripts/lib/pywheel.sh).
VERSION=0.12.0
TARBALL=https://files.pythonhosted.org/packages/c1/41/629e70c4cb32c1ddb88de970cd174bbb43d8241c8e07bdffc62a8280297c/argparse_addons-0.12.0-py3-none-any.whl
SHA256=48b70ecd719054fcb0d7e6f25a1fecc13607aac61d446e83f47d211b4ead0d61
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
