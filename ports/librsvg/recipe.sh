# librsvg: the SVG renderer of GNOME (Rust, with a C API) -- rsvg-convert,
# the library GTK programs draw SVG with (AbiWord, goffice/Gnumeric), and the
# gdk-pixbuf loader that makes every GdkPixbuf program read SVG: the symbolic
# and scalable icons of the icon themes, xfdesktop's SVG backgrounds.
# 2.60 is the newest series whose rust-version (1.89) the QNX toolchain of
# ports/rust (1.90.0) meets; 2.62 wants 1.92.
#
# Build machine tool: cargo-c (`cargo cbuild`, which makes the C library),
# installed once into hosttools/cargo-c by this recipe (_cargo_c), as ports/rust
# installs hosttools/rust: not a host tool of scripts/build-host-tools.sh,
# which runs before ports/rust exists. Version 0.10.19+cargo-0.93.0, the newest
# whose rust-version (1.90) the toolchain meets, from crates.io at a fixed
# checksum, with hosttools/cargo-c-0.10.19-qnx-nto.patch (cargo-c did not know
# target_os "nto": "The target nto-nto80 is not supported yet"); cargo install
# --locked, the crates in $HOME/.cargo (/build/home/.cargo in a neutral build).
# gdk-pixbuf's apk trigger lists the SVG loader in loaders.cache on install.
VERSION=2.60.3
TARBALL=https://download.gnome.org/sources/librsvg/2.60/librsvg-2.60.3.tar.xz
SHA256=ca7d206d37426147e92299a0e59acb5b5f1d8bd7da20ff23d7642885a89feab4
DEPS="rust glib cairo pango freetype harfbuzz libxml2 gdk-pixbuf"
RUNTIME_DEPS="gdk-pixbuf shared-mime-info"
BUILDSYS=meson
source "$WS/scripts/lib/rust.sh"
# - the Rust target is given (-Dtriplet): meson cannot derive a QNX one;
# - no introspection, Vala, docs or tests; no AVIF (dav1d);
# - the gdk-pixbuf loader is built (pixbuf-loader).
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Dvala=disabled -Ddocs=disabled -Dtests=false
                   -Davif=disabled -Dpixbuf=enabled -Dpixbuf-loader=enabled )
# Cargo builds the Rust part under meson. It fetches the crates Cargo.lock pins
# (CARGO_HOME under $HOME, as for zenoh-c); the libc crate in the lock (0.2.177)
# still knows QNX 8 as target_os "nto", target_env "nto80" (see ports/zenoh-c).
# The crates that link C libraries find them with pkg-config, in the staging
# tree: PKG_CONFIG_ALLOW_CROSS lets the pkg-config crate do that.
CARGO_C_VERSION=0.10.19+cargo-0.93.0
CARGO_C_SHA256=d42aa5310f659a1c3c0dea92f6dd5ba0d4e0d8d5c221ecc341ba36846ab5f7da
CARGO_C_HOME="$WS/hosttools/cargo-c"
_cargo_c() {
  [ -x "$CARGO_C_HOME/bin/cargo-cbuild" ] && return 0
  local c="$WS/dist/cargo-c-$CARGO_C_VERSION.crate" d="$BUILD_DIR/host/cargo-c"
  [ -f "$c" ] || curl -fsSL -o "$c" "https://static.crates.io/crates/cargo-c/cargo-c-$CARGO_C_VERSION.crate" || return 1
  echo "$CARGO_C_SHA256  $c" | sha256sum -c --status || { echo "checksum mismatch: $c"; rm -f "$c"; return 1; }
  rm -rf "$d" && mkdir -p "$d" && tar xzf "$c" -C "$d" --strip-components=1 &&
    patch -p1 -d "$d" < "$RECIPE_DIR/librsvg/hosttools/cargo-c-0.10.19-qnx-nto.patch" || return 1
  # a build machine program: none of the QNX cross environment
  env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u LDFLAGS -u CPPFLAGS -u PKG_CONFIG -u PKG_CONFIG_LIBDIR \
      -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH PATH="$WS/hosttools/rust/bin:/usr/bin:/bin" \
    cargo install --path "$d" --locked --root "$CARGO_C_HOME"
}
# Rust links the programs (rsvg-convert) with qcc: -rpath-link lets ld find the
# libraries cairo, pango, glib ... need in turn (libfribidi, libpcre2 ...),
# as the meson cross file does for C (otherwise "undefined reference to
# fribidi_get_bracket" from libpango). -L native=$B/rsvg: the gdk-pixbuf loader
# links the librsvg just built, which it finds through meson's uninstalled .pc
# -- whose -L names the build directory, and pkg-config puts the staging
# sysroot in front of that ("cannot find -lrsvg-2").
_rsvg_env() {
  local T; T=$(qnx_rust_triple | tr 'a-z-' 'A-Z_')
  qnx_rust_env env PKG_CONFIG_ALLOW_CROSS=1 PATH="$CARGO_C_HOME/bin:$WS/hosttools/rust/bin:$PATH" \
    "CARGO_TARGET_${T}_RUSTFLAGS=-C link-arg=-V$QCC_TARGET -C link-arg=-Wl,-rpath-link,$STAGE$PREFIX/lib -L native=$B/rsvg" "$@"
}
do_configure() {
  _cargo_c || return 1
  _rsvg_env \
    meson setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$QNX_ARCH.ini" \
      --prefix="$PREFIX" --buildtype=release -Dtriplet="$(qnx_rust_triple)" "${MESON_ARGS_EXTRA[@]}"
}
do_build() { _rsvg_env ninja -C "$B"; }
