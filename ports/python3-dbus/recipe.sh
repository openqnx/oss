# dbus-python: the Python bindings for libdbus (dbus.SessionBus, dbus.service
# exported objects, signals) with its GLib main loop integration
# (dbus.mainloop.glib, from the bundled dbus-gmain). Its C extensions
# (_dbus_bindings, _dbus_glib_bindings) are built with meson-python, cross
# compiled against the ported dbus and glib (scripts/lib/pywheel.sh).
VERSION=1.5.1
TARBALL=https://files.pythonhosted.org/packages/f3/cc/23f24bc7cbfec3a0ab508b7e11bc98837d77a91445d5f666a8545353a147/dbus_python-1.5.1.tar.gz
SHA256=57423cf87d09b7d084f420b55b737fe654c33339b06df77945620d97795c6ac4
DEPS="python3 dbus glib"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "meson-python>=0.16" "meson>=1.2" setuptools ninja "patchelf>=0.11.0" && qnx_py_crossfile; }
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" \
    -Csetup-args="--native-file=$B/qnx-native.ini" -Csetup-args="-Dtests=disabled" \
    -Csetup-args="-Ddoc=disabled" -Csetup-args="--wrap-mode=nodownload"
}
do_install() { qnx_py_install; }
