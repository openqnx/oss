# mousepad 0.6.5

> Xfce text editor: tabs, syntax highlighting, search and replace (GTK 3)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/apps/mousepad/0.6/mousepad-0.6.5.tar.xz |
| SHA-256 | `21762bc8c3c4f120a4a509ce39f4a5a58dbc10e3f0da66cdc6d9a8c735fff2ac` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`gtksourceview4`](../gtksourceview4/) [`libxfce4ui`](../libxfce4ui/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`gtksourceview4`](../gtksourceview4/) [`libxfce4ui`](../libxfce4ui/) [`pango`](../pango/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mousepad`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `mousepad` and `mousepad-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mousepad` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (62 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 gtksourceview4 libxfce4ui
```

<details><summary>Full build chain (62)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`gtksourceview4`](../gtksourceview4/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/apps/mousepad/0.6/mousepad-0.6.5.tar.xz
mkdir -p dist src/mousepad
curl -fL -o dist/$(basename $URL) $URL
echo "21762bc8c3c4f120a4a509ce39f4a5a58dbc10e3f0da66cdc6d9a8c735fff2ac  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mousepad --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/mousepad src/mousepad \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dgtksourceview4=enabled \
      -Dpolkit=disabled \
      -Dgspell-plugin=disabled \
      -Dshortcuts-plugin=enabled \
      -Dtest-plugin=disabled
ninja -C build/$QNX_ARCH/mousepad
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/mousepad install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- mousepad: the Xfce text editor (GTK 3, GtkSourceView 4) -- the plain text editor of Raspberry Pi OS's desktop: tabs, syntax highlighting, search and replace, encodings, recent files. Release series 0.6 (Xfce 4.20).
- libxfce4ui: the keyboard shortcut editor plugin.
- - GtkSourceView 4 (not 3, which is not ported); - no polkit (editing files of other users as root through pkexec: no polkit on QNX) and no gspell (the spell checker library is not ported); - settings in GSettings (dconf in the desktop), not the keyfile backend.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mousepad`.

### 5. What it installs

71 files under `/usr/local`: 3 shared-library entries, 1 programs.

```
/usr/local/lib/libmousepad.so
/usr/local/lib/libmousepad.so.0
/usr/local/lib/libmousepad.so.0.0.0
/usr/local/bin/mousepad
```

The complete list is `stage/<arch>/.manifest/mousepad` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mousepad
ls dists/8.0/*/ | grep '^mousepad-'
```

This writes `mousepad-<ver>-r0.tar.gz` (run time) and `mousepad-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mousepad
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mousepad-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mousepad-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mousepad`](../../tests/src/) links mousepad the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mousepad`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mousepad
/usr/local/bin/qnx-port-tests/test_mousepad
```

```
== mousepad ==
  ok   ...
PASS mousepad
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mousepad` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mousepad`](../../tests/src/) -- a program linked against mousepad the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> mousepad`.

## Versions

This recipe builds **0.6.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mousepad=<version>`), sources in `ported/mousepad/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mousepad`; port one with `scripts/port-update mousepad`.

<details><summary>recipe.sh</summary>

```bash
# mousepad: the Xfce text editor (GTK 3, GtkSourceView 4) -- the plain text
# editor of Raspberry Pi OS's desktop: tabs, syntax highlighting, search and
# replace, encodings, recent files. Release series 0.6 (Xfce 4.20).
VERSION=0.6.5
TARBALL=https://archive.xfce.org/src/apps/mousepad/0.6/mousepad-0.6.5.tar.xz
SHA256=21762bc8c3c4f120a4a509ce39f4a5a58dbc10e3f0da66cdc6d9a8c735fff2ac
# libxfce4ui: the keyboard shortcut editor plugin.
DEPS="glib gtk3 gtksourceview4 libxfce4ui"
RUNTIME_DEPS="gtksourceview4 libxfce4ui gsettings-desktop-schemas"
BUILDSYS=meson
# - GtkSourceView 4 (not 3, which is not ported);
# - no polkit (editing files of other users as root through pkexec: no polkit
#   on QNX) and no gspell (the spell checker library is not ported);
# - settings in GSettings (dconf in the desktop), not the keyfile backend.
MESON_ARGS_EXTRA=( -Dgtksourceview4=enabled -Dpolkit=disabled -Dgspell-plugin=disabled
                   -Dshortcuts-plugin=enabled -Dtest-plugin=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
