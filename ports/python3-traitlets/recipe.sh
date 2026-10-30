# traitlets: typed, validated, observable attributes (Jupyter's configuration system).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=5.16.1
TARBALL=https://files.pythonhosted.org/packages/ad/66/0d785f0bc5e4315a96c989bb476d0fc07ea4f85132550c7b156ca2035d52/traitlets-5.16.1-py3-none-any.whl
SHA256=f775618166caa0396c8e337099240f2bd3e5e917d203b2e6fbe21a58d3cb1f6b
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
