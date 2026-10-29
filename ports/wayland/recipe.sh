VERSION=1.23.1; GIT=https://gitlab.freedesktop.org/wayland/wayland.git; REF=1.23.1
DEPS="libffi expat libxml2"
BUILDSYS=meson
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
