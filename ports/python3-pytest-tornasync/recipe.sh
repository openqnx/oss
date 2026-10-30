# pytest-tornasync: pytest plugin running coroutine tests on tornado's IOLoop.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.6.0.post2
TARBALL=https://files.pythonhosted.org/packages/50/b9/3615ebfc3120bb949c3725b50793f42c3230d0175d6cfd358ea8bb6928ff/pytest_tornasync-0.6.0.post2-py3-none-any.whl
SHA256=4b165b6ba76b5b228933598f456b71ba233f127991a52889788db0a950ad04ba
DEPS="python3 python3-pytest python3-tornado"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-pytest python3-tornado"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
