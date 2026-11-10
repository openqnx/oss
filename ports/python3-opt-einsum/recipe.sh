# opt_einsum: optimised contraction order for numpy.einsum-style tensor
# expressions (used by JAX and PyTorch).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.4.0
TARBALL=https://files.pythonhosted.org/packages/23/cd/066e86230ae37ed0be70aae89aabf03ca8d9f39c8aea0dec8029455b5540/opt_einsum-3.4.0-py3-none-any.whl
SHA256=69bb92469f86a1565195ece4ac0323943e83477171b91d24c35afe028a90d7cd
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
