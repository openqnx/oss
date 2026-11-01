# PyYAML: YAML parser and emitter for Python (colcon-defaults, colcon-metadata).
# The pure-Python implementation: PYYAML_FORCE_LIBYAML=0 leaves out the
# optional libyaml-based C accelerator (yaml.CLoader), which would need
# Cython on the build machine; yaml.load/dump do not use it unless asked.
# Built from the source distribution into a platform-independent wheel
# (scripts/lib/pywheel.sh).
VERSION=6.0.3
TARBALL=https://files.pythonhosted.org/packages/05/8e/961c0007c59b8dd7729d542c61a4d537767a59645b82a0b521206e1e25c2/pyyaml-6.0.3.tar.gz
SHA256=d76623373421df22fb4cf8817020cbb7ef15c725b9d5e45f17e189bfc384190f
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_build() { PYYAML_FORCE_LIBYAML=0 qnx_py_wheel_pure; }
do_install() { qnx_py_install; }
