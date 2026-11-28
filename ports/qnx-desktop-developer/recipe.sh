# qnx-desktop-developer: building and debugging on the target -- Raspberry Pi OS'
# rpd-developer plus what its base image already carries. cc/c++ (clang set up
# for QNX: native-toolchain brings llvm, binutils, make and pkg-config), CMake,
# Ninja and Meson, Python 3 with pip, setuptools, wheel, virtualenv, NumPy and
# pygame, Git, GDB, Node.js with npm.
# The compiler needs QNX's headers and link libraries, which no package
# carries: scripts/target-sysroot copies them from an SDP (see native-toolchain).
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   a GPIO module for Python (gpiozero / RPi.GPIO counterpart on QNX's gpio-rp1)
#   libcamera tools and a Python camera module (libcamera is ported for aarch64le only)
#   python3-pyqt, python3-opengl
VERSION=1.0
DEPS="native-toolchain cmake ninja meson python3 python3-pip python3-setuptools python3-wheel python3-virtualenv numpy pygame git gdb nodejs npm"
RUNTIME_DEPS="native-toolchain cmake ninja meson python3 python3-pip python3-setuptools python3-wheel python3-virtualenv numpy pygame git gdb nodejs npm"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-developer/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/developer.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/developer.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/developer.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/developer.list"
}
