# gnumeric: the GNOME spreadsheet -- workbooks with formulas (the function
# library, solver, goal seek), charts (goffice), and the formats of other
# spreadsheets: Excel (.xls, .xlsx), OpenDocument (.ods), CSV/text, Lotus,
# Quattro Pro and more; ssconvert converts between them from the command line.
VERSION=1.12.62
TARBALL=https://download.gnome.org/sources/gnumeric/1.12/gnumeric-1.12.62.tar.xz
SHA256=89331121321b9bad72d37af5a6d13a0a636f0fbb0880f0ea1f2cf8b7ab9ae631
DEPS="glib gtk3 goffice libgsf libxml2 pango cairo zlib intltool gettext"
RUNTIME_DEPS="goffice gsettings-desktop-schemas"
BUILDSYS=autotools
# - no Perl or Python plugin loaders (scripting in the workbook), no GDA
#   (databases), Guile, Psiconv (Psion), Paradox, Mono, PDF manual;
# - no introspection (it cannot run QNX programs here).
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-perl --without-python --without-gda
                  --without-guile --without-psiconv --without-paradox --without-mono
                  --disable-pdfdocs --disable-introspection --disable-component )
# intltool as in ports/goffice and ports/xarchiver: wrappers run the staged
# Perl scripts with the build machine's perl, first on PATH throughout.
_gn_hostbin() {
  local t
  mkdir -p "$B/hostbin" || return 1
  for t in intltool-update intltool-merge intltool-extract; do
    printf '#!/bin/sh\nexec /usr/bin/perl "%s" "$@"\n' "$STAGE$PREFIX/bin/$t" > "$B/hostbin/$t" && chmod +x "$B/hostbin/$t" || return 1
  done
}
do_configure() {
  _gn_hostbin || return 1
  # ITSTOOL=/bin/true: itstool only translates the help pages and would have to
  # run on the build host (as ports/mate-calc); the English manual is installed.
  ( cd "$B" && PATH="$B/hostbin:$PATH" INTLTOOL_PERL=/usr/bin/perl ITSTOOL=/bin/true \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    LDFLAGS="-Wl,--push-state,--no-as-needed,-lc,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify $LDFLAGS_APPEND" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}" )
}
# glib-compile-resources runs at build time: the build host's, not the staged QNX
# one that gio-2.0.pc names (as ports/xfce4-panel). configure.ac sets it from
# pkg-config whatever the environment says, so it is given to make.
# HELP_LINGUAS=: the translated help pages are made by itstool (not run, see
# ITSTOOL above); only the English manual is installed.
_GN_MAKE=( GLIB_COMPILE_RESOURCES=/usr/bin/glib-compile-resources HELP_LINGUAS= )
do_build() { PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)" "${_GN_MAKE[@]}"; }
do_install() { PATH="$B/hostbin:$PATH" make -C "$B" "${_GN_MAKE[@]}" DESTDIR="$STAGE" install; }
