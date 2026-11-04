# cattrs: structuring and unstructuring of attrs classes and dataclasses.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=26.2.1
TARBALL=https://files.pythonhosted.org/packages/e8/cf/22794a399d99480486120e26e879ef008e21f5e85274c2ed591d568bb326/cattrs-26.2.1-py3-none-any.whl
SHA256=a12aaa3453dc8f633a815293179f08b7421ed18d2575c459c3c736f840beac24
DEPS="python3 python3-attrs python3-typing-extensions"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-attrs python3-typing-extensions"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
