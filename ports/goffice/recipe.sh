# goffice: the GNOME office library under Gnumeric -- charts and graphs
# (plot engines, the graph editor), the canvas, formats, number formatting,
# component plugins and the GUI helpers office programs share.
VERSION=0.10.62
TARBALL=https://download.gnome.org/sources/goffice/0.10/goffice-0.10.62.tar.xz
SHA256=500eaff50628faa75adb3cd560236a4db498d9e7c52c036b78fa9a57f21805b3
DEPS="glib libgsf libxml2 pango cairo gdk-pixbuf libxslt gtk3 librsvg intltool gettext"
RUNTIME_DEPS="librsvg gsettings-desktop-schemas"
BUILDSYS=autotools
# - SVG through librsvg (pictures in charts and documents); no lasem (MathML
#   and SVG equations: not ported);
# - settings in GSettings; no introspection (it cannot run QNX programs here).
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-gtk --with-librsvg --without-lasem
                  --with-config-backend=gsettings --disable-introspection )
# intltool (configure checks it; po/ is made with it): the staged programs are
# the target's Perl scripts (#!/usr/local/bin/perl); wrappers in the build
# directory run them with the build machine's perl, first on PATH for
# configure, make and the install -- as ports/xarchiver, whose comments say
# why setting INTLTOOL_UPDATE alone does not do. The cross environment is
# spelled out because a hook replaces the whole step.
_go_hostbin() {
  local t
  mkdir -p "$B/hostbin" || return 1
  for t in intltool-update intltool-merge intltool-extract; do
    printf '#!/bin/sh\nexec /usr/bin/perl "%s" "$@"\n' "$STAGE$PREFIX/bin/$t" > "$B/hostbin/$t" && chmod +x "$B/hostbin/$t" || return 1
  done
}
do_configure() {
  _go_hostbin || return 1
  ( cd "$B" && PATH="$B/hostbin:$PATH" INTLTOOL_PERL=/usr/bin/perl \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    LDFLAGS="-Wl,--push-state,--no-as-needed,-lc,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify $LDFLAGS_APPEND" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}" )
}
do_build() { PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)"; }
do_install() { PATH="$B/hostbin:$PATH" make -C "$B" DESTDIR="$STAGE" install; }
