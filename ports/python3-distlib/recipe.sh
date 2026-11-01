# distlib: distribution utilities.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.4.4
TARBALL=https://files.pythonhosted.org/packages/c1/2b/238f02a4927fa2e27aa995371d240a468b52e42eae44a678434aff736b38/distlib-0.4.4-py2.py3-none-any.whl
SHA256=cf09d55b1e85a89576b036aeb71550bfd68320f3f21761ca329eadadad918638
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
