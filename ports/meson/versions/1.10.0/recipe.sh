# Meson, the build system, for the target (runs on the target's Python).
# Pure Python: the published wheel is installed as released. Version as in the
# QNX Everywhere 8.0.4r1 image.
VERSION=1.10.0
TARBALL=https://files.pythonhosted.org/packages/32/4f/c398c6f06ece1c6c246e008d5dac3824c98f54d3eb3d8014f4910afd6d48/meson-1.10.0-py3-none-any.whl
SHA256=4b27aafce281e652dcb437b28007457411245d975c48b5db3a797d3e93ae1585
DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
# The wheel's console script entry point is written as a small launcher.
do_install() { qnx_py_install_pure && install -d "$STAGE$PREFIX/bin" && \
  printf '#!/usr/local/bin/python3\nimport sys\nfrom mesonbuild.mesonmain import main\nsys.exit(main())\n' > "$STAGE$PREFIX/bin/meson" && \
  chmod 755 "$STAGE$PREFIX/bin/meson"; }
