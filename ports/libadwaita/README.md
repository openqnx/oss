# libadwaita 1.6.10

> Building blocks for modern GNOME applications

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/libadwaita/1.6/libadwaita-1.6.10.tar.xz |
| SHA-256 | `714a486e748e1b7de29101436fba3bbb61f74997f373c6af9651e23e859eb1f6` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk4`](../gtk4/) [`fribidi`](../fribidi/) [`appstream`](../appstream/) |
| Run-time dependencies | [`appstream`](../appstream/) [`fribidi`](../fribidi/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphene`](../graphene/) [`gtk4`](../gtk4/) [`pango`](../pango/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libadwaita`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libadwaita` (programs, shared libraries, run-time data) and `libadwaita-dev` (headers, pkg-config/CMake files, static libraries; needs `libadwaita`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libadwaita`](https://openqnx.github.io/oss/package/x86_64/libadwaita.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libadwaita.html)) · [`libadwaita-dev`](https://openqnx.github.io/oss/package/x86_64/libadwaita-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libadwaita-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libadwaita` (dependencies come along); one version: `apk add libadwaita=<version>` | `apk del libadwaita` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libadwaita` (dependencies first) | `oss-get remove libadwaita` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libadwaita-dev` the same way, by its own name (`apk add libadwaita-dev`, `oss-get install libadwaita-dev`, by hand with its own file and list `libadwaita-dev.list`); remove it before `libadwaita`, which it needs.

```sh
# on the QNX target, as root
apk add libadwaita libadwaita-dev
apk add qnx-test-libadwaita && /usr/local/bin/qnx-port-tests/test_libadwaita    # its on-target test, from tests-8.0-<arch>
apk del libadwaita libadwaita-dev qnx-test-libadwaita
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libadwaita.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libadwaita.list
rm var/lib/manual/libadwaita.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libadwaita` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (58 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk4 fribidi appstream
```

<details><summary>Full build chain (58)</summary>

[`appstream`](../appstream/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`cairo`](../cairo/) [`curl`](../curl/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`gtk4`](../gtk4/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxmlb`](../libxmlb/) [`libxrender`](../libxrender/) [`libyaml`](../libyaml/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/libadwaita/1.6/libadwaita-1.6.10.tar.xz
mkdir -p dist src/libadwaita
curl -fL -o dist/$(basename $URL) $URL
echo "714a486e748e1b7de29101436fba3bbb61f74997f373c6af9651e23e859eb1f6  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libadwaita --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libadwaita; B=build/$QNX_ARCH/libadwaita; mkdir -p $B
```

```bash
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
```

```bash
do_configure() {
  PATH="$SRCROOT/qnx-native/bin:$PATH" \
  meson setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$arch.ini" \
    --native-file "$QNX_MESON_DIR/meson-native.ini" \
    --prefix="$PREFIX" --buildtype=release --default-library=both "${MESON_ARGS_EXTRA[@]}"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libadwaita: building blocks for modern GNOME applications on GTK 4 (adaptive widgets, the Adwaita style sheet, AdwApplication, dialogs, toasts, tab views, preferences pages). Dependency of libpanel. 1.6: the newest series that builds against the gtk4 component (4.16; libadwaita 1.7 needs GTK 4.17.5), and what libpanel 1.10 asks for.
- The style sheets are compiled from SCSS at build time with sassc, a program the *build machine* runs; the release tarball carries no compiled CSS, and meson's fallback (the sassc subproject) would be cross-compiled for QNX and could not run. sassc and libsass are therefore built natively first, from their release archives (checksums pinned here), into the source tree, and put first on PATH for the configuration.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libadwaita`.

### 5. What it installs

133 files under `/usr/local`: 2 shared-library entries, 79 headers.

```
/usr/local/lib/libadwaita-1.so
/usr/local/lib/libadwaita-1.so.0
```

The complete list is `stage/<arch>/.manifest/libadwaita` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libadwaita
ls dists/8.0/*/ | grep '^libadwaita-'
```

This writes `libadwaita-<ver>-r0.tar.gz` (run time) and `libadwaita-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libadwaita
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libadwaita-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libadwaita-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libadwaita`](../../tests/src/) links libadwaita the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libadwaita`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libadwaita
/usr/local/bin/qnx-port-tests/test_libadwaita
```

```
== libadwaita ==
  ok   ...
PASS libadwaita
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libadwaita` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libadwaita`](../../tests/src/) -- a program linked against libadwaita the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libadwaita 1.6
   - breakpoint condition parsed and printed back
   - adw_lerp(10, 20, 0.25) = 12.5
   - spring params: damping ratio 1, mass 1, stiffness 100 -> damping 20
   - labwc (headless) Wayland socket appeared
   - gtk_init_check on labwc
   - adw_init
   - 72 pt = 96 px
   - Adwaita style sheet (compiled from SCSS) is in the library
   - style manager: FORCE_DARK makes it dark
   - style manager: FORCE_LIGHT makes it light
   - AdwApplicationWindow mapped on the compositor
   - view stack shows page two
   - tab view with three pages
   - closing a tab leaves two
   - toast shown
   - window was allocated a size
   - AdwAboutDialog from AppStream metainfo
   - metainfo: name, newest version, developer
   - metainfo: license type and issue URL
   - metainfo: release notes of 2.1

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libadwaita 1.6.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libadwaita-1.6.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libadwaita 1.6.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libadwaita-1.6.10-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.6.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libadwaita=<version>`), sources in `ported/libadwaita/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libadwaita`; port one with `scripts/port-update libadwaita`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
