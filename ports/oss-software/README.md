# oss-software 1.0

> Add or Remove Software: a GTK 3 front end to apk (browse, install, remove, update)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`gtk3`](../gtk3/) [`glib`](../glib/) [`json-glib`](../json-glib/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`json-glib`](../json-glib/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_oss_software`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `oss-software` and `oss-software-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild oss-software` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (59 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gtk3 glib json-glib
```

<details><summary>Full build chain (59)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`json-glib`](../json-glib/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-software/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/oss-software; B=build/$QNX_ARCH/oss-software; mkdir -p $B
```

```bash
do_build() {
  mkdir -p "$B" && cd "$B" && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -o oss-software "$S/oss-software.c" \
      $(pkg-config --cflags --libs gtk+-3.0 gio-unix-2.0 json-glib-1.0)
}
```

```bash
do_install() {
  install -Dm755 "$B/oss-software" "$STAGE$PREFIX/bin/oss-software" && \
  install -Dm644 "$S/org.openqnx.Software.desktop" \
                 "$STAGE$PREFIX/share/applications/org.openqnx.Software.desktop"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- oss-software: "Add or Remove Software" for the desktop -- a GTK 3 front end to apk-tools (the place of Raspberry Pi OS' rp-prefapps / gui-pkinst): browse and search the repository, install, remove, check for updates and update everything, with a progress bar. Written for this tree (files/). apk runs as a child process (GSubprocess), never in the window's thread: the lists from `apk query --format json` (read with json-glib), the progress of an operation from --progress-fd, apk's own error lines in a dialog when it fails; while another apk holds the database lock it waits and says so. C rather than Python: GTK has no GObject introspection data in this tree (see GAP.md), which PyGObject would need. The desktop entry is named after the application id (org.openqnx.Software): that is the Wayland app_id the panel's window buttons look it up by.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH oss-software`.

### 5. What it installs

2 files under `/usr/local`, 1 programs.

```
/usr/local/bin/oss-software
```

The complete list is `stage/<arch>/.manifest/oss-software` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa oss-software
ls dists/8.0/*/ | grep '^oss-software-'
```

This writes `oss-software-<ver>-r0.tar.gz` (run time) and `oss-software-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add oss-software
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/oss-software-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/oss-software-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_oss_software`](../../tests/src/) links oss-software the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-oss-software`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-oss-software
/usr/local/bin/qnx-port-tests/test_oss_software
```

```
== oss-software ==
  ok   ...
PASS oss-software
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_oss_software` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_oss_software`](../../tests/src/) -- a program linked against oss-software the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> oss-software`.

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add oss-software=<version>`), sources in `ported/oss-software/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates oss-software`; port one with `scripts/port-update oss-software`.

<details><summary>recipe.sh</summary>

```bash
# oss-software: "Add or Remove Software" for the desktop -- a GTK 3 front end
# to apk-tools (the place of Raspberry Pi OS' rp-prefapps / gui-pkinst):
# browse and search the repository, install, remove, check for updates and
# update everything, with a progress bar. Written for this tree (files/).
#
# apk runs as a child process (GSubprocess), never in the window's thread:
# the lists from `apk query --format json` (read with json-glib), the
# progress of an operation from --progress-fd, apk's own error lines in a
# dialog when it fails; while another apk holds the database lock it waits
# and says so. C rather than Python: GTK has no GObject introspection data
# in this tree (see GAP.md), which PyGObject would need.
# The desktop entry is named after the application id (org.openqnx.Software):
# that is the Wayland app_id the panel's window buttons look it up by.
VERSION=1.0
DEPS="gtk3 glib json-glib"
RUNTIME_DEPS="apk-tools adwaita-icon-theme"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-software/files/." "$SRCROOT/"; }
do_build() {
  mkdir -p "$B" && cd "$B" && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -o oss-software "$S/oss-software.c" \
      $(pkg-config --cflags --libs gtk+-3.0 gio-unix-2.0 json-glib-1.0)
}
do_install() {
  install -Dm755 "$B/oss-software" "$STAGE$PREFIX/bin/oss-software" && \
  install -Dm644 "$S/org.openqnx.Software.desktop" \
                 "$STAGE$PREFIX/share/applications/org.openqnx.Software.desktop"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
