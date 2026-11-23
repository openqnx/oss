# pytz (run-time dependency of pandas).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=2026.6
TARBALL=https://files.pythonhosted.org/packages/b6/25/564f3fc5b70c1398d706c51f2b1470d0ea0b1b2f96fee67877b6f5bdf3b6/pytz-2026.6-py2.py3-none-any.whl
SHA256=7fed3fee8fd6bd4a2b7b75a8d56021d0624127bf69bd5691f115db929ac570b3
DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
