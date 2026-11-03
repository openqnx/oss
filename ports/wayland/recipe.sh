VERSION=1.24.0; GIT=https://gitlab.freedesktop.org/wayland/wayland.git; REF=1.24.0
DEPS="libffi expat libxml2"
BUILDSYS=meson
# QNX's Wayland EGL (/system/lib/libEGL.so.1 on graphics images) dlopen()s the
# unversioned names libwayland-client.so and libwayland-server.so (implib-gen
# shims; it never asks for libwayland-egl). Those are the -dev links, so
# without wayland-dev every EGL-on-Wayland client (GTK 4, wlroots) failed:
# "Failed to load wayland client shared object: libwayland-client.so". The
# runtime package keeps these two links (qnxpkg: RUNTIME_SOLINKS). Links, not
# copies: QNX's loader maps a symlinked name onto the object already loaded,
# while a copy under the unversioned name is loaded a second time (other
# wl_*_interface addresses; checked with dlopen/dlsym on the x86_64 VM).
RUNTIME_SOLINKS="libwayland-client.so libwayland-server.so"
# wayland-scanner has to run on the build machine to turn the XML protocol
# definitions into C; hosttools/ supplies a native one.
MESON_ARGS_EXTRA=( -Dtests=false -Ddocumentation=false -Ddtd_validation=true
                   -Dscanner=false -Dlibraries=true )

# -Dscanner=false means no wayland-scanner.pc is installed either, and packages
# ask for the scanner through pkg-config by name -- libxfce4windowing's configure
# stops at "dependencies missing: wayland-scanner >= 1.20". Publish a .pc in the
# staging tree that points at the native scanner in hosttools/, which is the one
# that must actually run.
do_install() {
  DESTDIR="$STAGE" ninja -C "$B" install || return 1
  local pc="$STAGE$PREFIX/lib/pkgconfig/wayland-scanner.pc"
  mkdir -p "$(dirname "$pc")"
  cat > "$pc" <<PC
prefix=$WS/hosttools
datarootdir=\${prefix}/share
pkgdatadir=\${datarootdir}/wayland
bindir=\${prefix}/bin
wayland_scanner=\${bindir}/wayland-scanner

Name: Wayland Scanner
Description: Wayland scanner (build machine binary, for cross builds)
Version: $VERSION
PC
}
