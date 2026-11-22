# platformdirs: the platform's user and site data/config/cache directories.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=4.14.8
TARBALL=https://files.pythonhosted.org/packages/5b/be/373cc7341f53bfd3acba3e5db62433fd960a45fcf73ad64a5f4d27e74910/platformdirs-4.14.8-py3-none-any.whl
SHA256=1e34af590de4d978631b751eb2202f2fee4497537a17c1720b12fcf57b331392
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
