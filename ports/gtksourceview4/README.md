# gtksourceview4 4.8.4

> GTK 3 source-code editing widget: syntax highlighting, line numbers, search and undo (GtkSourceView 4)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/gtksourceview/4.8/gtksourceview-4.8.4.tar.xz |
| SHA-256 | `7ec9d18fb283d1f84a3a3eff3b7a72b09a10c9c006597b3fbabbb5958420a87d` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxml2`](../libxml2/) [`fribidi`](../fribidi/) [`pango`](../pango/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxml2`](../libxml2/) [`pango`](../pango/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gtksourceview4`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `gtksourceview4` and `gtksourceview4-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gtksourceview4` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib gtk3 libxml2 fribidi pango
```

<details><summary>Full build chain (58)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/gtksourceview/4.8/gtksourceview-4.8.4.tar.xz
mkdir -p dist src/gtksourceview4
curl -fL -o dist/$(basename $URL) $URL
echo "7ec9d18fb283d1f84a3a3eff3b7a72b09a10c9c006597b3fbabbb5958420a87d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gtksourceview4 --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gtksourceview4 src/gtksourceview4 \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dgir=false \
      -Dvapi=false \
      -Dgtk_doc=false \
      -Dglade_catalog=false \
      -Dinstall_tests=false
ninja -C build/$QNX_ARCH/gtksourceview4
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gtksourceview4 install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- gtksourceview4: GtkSourceView 4, the GTK 3 source-code editing widget (syntax highlighting, line numbers, search, undo, style schemes) -- the text area of Mousepad and other GTK 3 editors. The 4.x series is the GTK 3 one (5.x is for GTK 4).
- No GObject introspection data, Vala bindings, API docs, Glade catalog or installed tests.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gtksourceview4`.

### 5. What it installs

311 files under `/usr/local`: 3 shared-library entries, 42 headers.

```
/usr/local/lib/libgtksourceview-4.so
/usr/local/lib/libgtksourceview-4.so.0
/usr/local/lib/libgtksourceview-4.so.0.0.0
```

The complete list is `stage/<arch>/.manifest/gtksourceview4` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gtksourceview4
ls dists/8.0/*/ | grep '^gtksourceview4-'
```

This writes `gtksourceview4-<ver>-r0.tar.gz` (run time) and `gtksourceview4-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gtksourceview4
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gtksourceview4-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gtksourceview4-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gtksourceview4`](../../tests/src/) links gtksourceview4 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gtksourceview4`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gtksourceview4
/usr/local/bin/qnx-port-tests/test_gtksourceview4
```

```
== gtksourceview4 ==
  ok   ...
PASS gtksourceview4
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gtksourceview4` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gtksourceview4`](../../tests/src/) -- a program linked against gtksourceview4 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> gtksourceview4`.

## Versions

This recipe builds **4.8.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gtksourceview4=<version>`), sources in `ported/gtksourceview4/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gtksourceview4`; port one with `scripts/port-update gtksourceview4`.

<details><summary>recipe.sh</summary>

```bash
# gtksourceview4: GtkSourceView 4, the GTK 3 source-code editing widget
# (syntax highlighting, line numbers, search, undo, style schemes) -- the text
# area of Mousepad and other GTK 3 editors. The 4.x series is the GTK 3 one
# (5.x is for GTK 4).
VERSION=4.8.4
TARBALL=https://download.gnome.org/sources/gtksourceview/4.8/gtksourceview-4.8.4.tar.xz
SHA256=7ec9d18fb283d1f84a3a3eff3b7a72b09a10c9c006597b3fbabbb5958420a87d
DEPS="glib gtk3 libxml2 fribidi pango"
BUILDSYS=meson
# No GObject introspection data, Vala bindings, API docs, Glade catalog or
# installed tests.
MESON_ARGS_EXTRA=( -Dgir=false -Dvapi=false -Dgtk_doc=false -Dglade_catalog=false -Dinstall_tests=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
