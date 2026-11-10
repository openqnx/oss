# flake8-blind-except: flake8 plugin for bare and blind except clauses
# (B901 "except:", B902 "except Exception:").
# Pure Python, published only as a source distribution: the build
# machine's Python builds the platform independent wheel (scripts/lib/pywheel.sh).
VERSION=0.2.1
TARBALL=https://files.pythonhosted.org/packages/be/f8/cb5e0b8c948cb347b8942cb7ce73292a77ec8f9eafb8d5db999c5c1ac62f/flake8-blind-except-0.2.1.tar.gz
SHA256=f25a575a9dcb3eeb3c760bf9c22db60b8b5a23120224ed1faa9a43f75dd7dd16
DEPS="python3 python3-flake8"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-flake8"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_build() { qnx_py_wheel_pure; }
do_install() { qnx_py_install; }
