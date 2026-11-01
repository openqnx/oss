# libadwaita: building blocks for modern GNOME applications on GTK 4
# (adaptive widgets, the Adwaita style sheet, AdwApplication, dialogs,
# toasts, tab views, preferences pages). Dependency of libpanel.
# 1.6: the newest series that builds against the gtk4 component (4.16;
# libadwaita 1.7 needs GTK 4.17.5), and what libpanel 1.10 asks for.
VERSION=1.6.10
TARBALL=https://download.gnome.org/sources/libadwaita/1.6/libadwaita-1.6.10.tar.xz
SHA256=714a486e748e1b7de29101436fba3bbb61f74997f373c6af9651e23e859eb1f6
DEPS="glib gtk4 fribidi appstream"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Dvapi=false -Dgtk_doc=false -Dtests=false -Dexamples=false )
# The style sheets are compiled from SCSS at build time with sassc, a program
# the *build machine* runs; the release tarball carries no compiled CSS, and
# meson's fallback (the sassc subproject) would be cross-compiled for QNX
# and could not run. sassc and libsass are therefore built natively first,
# from their release archives (checksums pinned here), into the source tree,
# and put first on PATH for the configuration.
_NATIVE_SRCS=(
  "https://github.com/sass/libsass/archive/refs/tags/3.6.6.tar.gz libsass-3.6.6.tar.gz 11f0bb3709a4f20285507419d7618f3877a425c0131ea8df40fe6196129df15d"
  "https://github.com/sass/sassc/archive/refs/tags/3.6.2.tar.gz sassc-3.6.2.tar.gz 608dc9002b45a91d11ed59e352469ecc05e4f58fc1259fc9a9f5b8f0f8348a03"
)
do_patch() {
  local n="$SRCROOT/qnx-native" e url f sum
  mkdir -p "$n/bin" "$WS/dist" || return 1
  for e in "${_NATIVE_SRCS[@]}"; do
    read -r url f sum <<<"$e"
    [ -f "$WS/dist/$f" ] || curl -fsSL --retry 3 -o "$WS/dist/$f" "$url" || return 1
    echo "$sum  $WS/dist/$f" | sha256sum -c --status || { echo "checksum mismatch: $f"; rm -f "$WS/dist/$f"; return 1; }
    mkdir -p "$n/${f%.tar.gz}" && tar xzf "$WS/dist/$f" -C "$n/${f%.tar.gz}" --strip-components=1 || return 1
  done
  # Plain host compilers: the QNX environment's CC/CXX must not leak in.
  env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u LDFLAGS -u CPPFLAGS \
    make -C "$n/libsass-3.6.6" -j"$(nproc)" CC=gcc CXX=g++ static &&
  env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u LDFLAGS -u CPPFLAGS \
    make -C "$n/sassc-3.6.2" -j"$(nproc)" CC=gcc CXX=g++ SASS_LIBSASS_PATH="$n/libsass-3.6.6" &&
  cp "$n/sassc-3.6.2/bin/sassc" "$n/bin/sassc"
}
do_configure() {
  PATH="$SRCROOT/qnx-native/bin:$PATH" \
  meson setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$arch.ini" \
    --native-file "$QNX_MESON_DIR/meson-native.ini" \
    --prefix="$PREFIX" --buildtype=release --default-library=both "${MESON_ARGS_EXTRA[@]}"
}
