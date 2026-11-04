# dbus-glib: the legacy GLib bindings for D-Bus (DBusGConnection,
# DBusGProxy, the dbus-binding-tool code generator). Deprecated upstream in
# favour of GDBus; kept for older software that still uses it.
VERSION=0.116
TARBALL=https://dbus.freedesktop.org/releases/dbus-glib/dbus-glib-0.116.tar.gz
SHA256=e3f3d4487e2883800770ed5899ed111bdc4ba7056af34a255c4c46ad8a2486f3
DEPS="dbus glib expat"
BUILDSYS=autotools
# The build generates dbus-glib-bindings.h by running dbus-binding-tool,
# which a cross build can only do with a native copy (hazard 65): build one
# from the same source against the build machine's dbus-1/glib-2.0 first,
# and point configure at it. Tests and the bash-completion helper are off.
_DBG_NATIVE="$BUILD_DIR/native-dbus-glib"
CONF_ARGS_EXTRA=( --disable-static --disable-tests --disable-bash-completion
                  --with-dbus-binding-tool="$_DBG_NATIVE/dbus/dbus-binding-tool" )
_dbg_native() {
  [ -x "$_DBG_NATIVE/dbus/dbus-binding-tool" ] && return 0
  rm -rf "$_DBG_NATIVE" && mkdir -p "$_DBG_NATIVE" && cd "$_DBG_NATIVE" &&
  env -u PKG_CONFIG -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH \
      PATH=/usr/bin:/bin CC=gcc CFLAGS=-O1 "$S/configure" --disable-tests --disable-bash-completion --disable-static &&
  env PATH=/usr/bin:/bin make -C dbus-gmain -j"$(nproc)" &&
  env PATH=/usr/bin:/bin make -C dbus -j"$(nproc)" dbus-binding-tool
}
do_build() { ( _dbg_native ) && make -C "$B" -j"$(nproc)"; }
