# PyYAML: YAML parser and emitter for Python (colcon-defaults, colcon-metadata).
# The pure-Python implementation: PYYAML_FORCE_LIBYAML=0 leaves out the
# optional libyaml-based C accelerator (yaml.CLoader), which would need
# Cython on the build machine; yaml.load/dump do not use it unless asked.
# Built from the source distribution into a platform-independent wheel
# (scripts/lib/pywheel.sh).
VERSION=6.0.4
TARBALL=https://files.pythonhosted.org/packages/76/b8/c0b780928e2fb1c87e9e375818c6f121da2398b071012d1747be300b83f4/pyyaml-6.0.4.tar.gz
SHA256=e24bf71af8a432d7808fabff134d38dfe16d3cbb41f2873bb4b1e42422de0a1a
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_build() { PYYAML_FORCE_LIBYAML=0 qnx_py_wheel_pure; }
do_install() { qnx_py_install; }
