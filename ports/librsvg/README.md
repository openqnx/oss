# librsvg 2.60.3

> SVG rendering library (Rust) with rsvg-convert and the gdk-pixbuf SVG loader

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/librsvg/2.60/librsvg-2.60.3.tar.xz |
| SHA-256 | `ca7d206d37426147e92299a0e59acb5b5f1d8bd7da20ff23d7642885a89feab4` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`rust`](../rust/) [`glib`](../glib/) [`cairo`](../cairo/) [`pango`](../pango/) [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) [`libxml2`](../libxml2/) [`gdk-pixbuf`](../gdk-pixbuf/) |
| Run-time dependencies | [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`glib`](../glib/) [`libxml2`](../libxml2/) [`pango`](../pango/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_librsvg`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `librsvg` and `librsvg-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild librsvg` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (44 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild rust glib cairo pango freetype harfbuzz libxml2 gdk-pixbuf
```

<details><summary>Full build chain (44)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`rust`](../rust/) [`sqlite`](../sqlite/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/librsvg/2.60/librsvg-2.60.3.tar.xz
mkdir -p dist src/librsvg
curl -fL -o dist/$(basename $URL) $URL
echo "ca7d206d37426147e92299a0e59acb5b5f1d8bd7da20ff23d7642885a89feab4  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/librsvg --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/librsvg; B=build/$QNX_ARCH/librsvg; mkdir -p $B
```

```bash
do_configure() {
  _cargo_c || return 1
  _rsvg_env \
    meson setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$QNX_ARCH.ini" \
      --prefix="$PREFIX" --buildtype=release -Dtriplet="$(qnx_rust_triple)" "${MESON_ARGS_EXTRA[@]}"
}
```

```bash
do_build() { _rsvg_env ninja -C "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- librsvg: the SVG renderer of GNOME (Rust, with a C API) -- rsvg-convert, the library GTK programs draw SVG with (AbiWord, goffice/Gnumeric), and the gdk-pixbuf loader that makes every GdkPixbuf program read SVG: the symbolic and scalable icons of the icon themes, xfdesktop's SVG backgrounds. 2.60 is the newest series whose rust-version (1.89) the QNX toolchain of ports/rust (1.90.0) meets; 2.62 wants 1.92. Build machine tool: cargo-c (`cargo cbuild`, which makes the C library), installed once into hosttools/cargo-c by this recipe (_cargo_c), as ports/rust installs hosttools/rust: not a host tool of scripts/build-host-tools.sh, which runs before ports/rust exists. Version 0.10.19+cargo-0.93.0, the newest whose rust-version (1.90) the toolchain meets, from crates.io at a fixed checksum, with hosttools/cargo-c-0.10.19-qnx-nto.patch (cargo-c did not know target_os "nto": "The target nto-nto80 is not supported yet"); cargo install --locked, the crates in $HOME/.cargo (/build/home/.cargo in a neutral build). gdk-pixbuf's apk trigger lists the SVG loader in loaders.cache on install.
- - the Rust target is given (-Dtriplet): meson cannot derive a QNX one; - no introspection, Vala, docs or tests; no AVIF (dav1d); - the gdk-pixbuf loader is built (pixbuf-loader).
- Cargo builds the Rust part under meson. It fetches the crates Cargo.lock pins (CARGO_HOME under $HOME, as for zenoh-c); the libc crate in the lock (0.2.177) still knows QNX 8 as target_os "nto", target_env "nto80" (see ports/zenoh-c). The crates that link C libraries find them with pkg-config, in the staging tree: PKG_CONFIG_ALLOW_CROSS lets the pkg-config crate do that.
- Rust links the programs (rsvg-convert) with qcc: -rpath-link lets ld find the libraries cairo, pango, glib ... need in turn (libfribidi, libpcre2 ...), as the meson cross file does for C (otherwise "undefined reference to fribidi_get_bracket" from libpango). -L native=$B/rsvg: the gdk-pixbuf loader links the librsvg just built, which it finds through meson's uninstalled .pc -- whose -L names the build directory, and pkg-config puts the staging sysroot in front of that ("cannot find -lrsvg-2").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH librsvg`.

### 5. What it installs

12 files under `/usr/local`: 3 shared-library entries, 1 programs, 5 headers.

```
/usr/local/lib/librsvg-2.so
/usr/local/lib/librsvg-2.so.2
/usr/local/lib/librsvg-2.so.2.60.3
/usr/local/bin/rsvg-convert
```

The complete list is `stage/<arch>/.manifest/librsvg` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa librsvg
ls dists/8.0/*/ | grep '^librsvg-'
```

This writes `librsvg-<ver>-r0.tar.gz` (run time) and `librsvg-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add librsvg
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/librsvg-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/librsvg-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_librsvg`](../../tests/src/) links librsvg the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-librsvg`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-librsvg
/usr/local/bin/qnx-port-tests/test_librsvg
```

```
== librsvg ==
  ok   ...
PASS librsvg
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_librsvg` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_librsvg`](../../tests/src/) -- a program linked against librsvg the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> librsvg`.

## Versions

This recipe builds **2.60.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add librsvg=<version>`), sources in `ported/librsvg/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates librsvg`; port one with `scripts/port-update librsvg`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
