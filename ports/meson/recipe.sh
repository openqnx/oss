# Meson, the build system, for the target (runs on the target's Python).
# Pure Python: the published wheel is installed as released. Version as in the
# QNX Everywhere 8.0.4r1 image.
VERSION=1.12.1
TARBALL=https://files.pythonhosted.org/packages/57/99/36cd25f9598eef70db97577440a70a360af505f6f01c2338a18d1556079c/meson-1.12.1-py3-none-any.whl
SHA256=930bc7542cbd9f57009e182fd014eba48cf1a0180a7b9006d2d32d8f168d8b02
DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
# The wheel's console script entry point is written as a small launcher.
do_install() { qnx_py_install_pure && install -d "$STAGE$PREFIX/bin" && \
  printf '#!/usr/local/bin/python3\nimport sys\nfrom mesonbuild.mesonmain import main\nsys.exit(main())\n' > "$STAGE$PREFIX/bin/meson" && \
  chmod 755 "$STAGE$PREFIX/bin/meson"; }
