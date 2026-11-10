# snowballstemmer: Snowball stemming algorithms for many languages, in
# pure Python (pydocstyle's imperative-mood check).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=3.1.1
TARBALL=https://files.pythonhosted.org/packages/4c/07/2ebca9b11fb9be7340a818d8d6f63feaebb146be2c4afbd6061701d6df6e/snowballstemmer-3.1.1-py3-none-any.whl
SHA256=7e207fa178741da09cdee59d3ecec3827ad5f92b1fc5c9ff3755b639f71f5752
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
