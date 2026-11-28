# oss-greeter 1.0

> Log-in screen for the QNX desktop (users, password through PAM, restart, shut down)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`gtk3`](../gtk3/) [`gtk-layer-shell`](../gtk-layer-shell/) [`openpam`](../openpam/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`openpam`](../openpam/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_oss_greeter`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `oss-greeter` and `oss-greeter-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild oss-greeter` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (60 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gtk3 gtk-layer-shell openpam
```

<details><summary>Full build chain (60)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openpam`](../openpam/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-greeter/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/oss-greeter; B=build/$QNX_ARCH/oss-greeter; mkdir -p $B
```

```bash
do_build() {
  mkdir -p "$B" && cd "$B" && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -DPREFIX="\"$PREFIX\"" -o oss-greeter "$S/oss-greeter.c" \
      $(pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0) -lpam
}
```

```bash
do_install() {
  install -Dm755 "$B/oss-greeter" "$STAGE$PREFIX/libexec/oss-greeter/oss-greeter" && \
  install -Dm755 "$S/oss-greeter-session" "$STAGE$PREFIX/libexec/oss-greeter/oss-greeter-session" && \
  install -Dm644 "$S/labwc/rc.xml" "$STAGE$PREFIX/share/oss-greeter/labwc/rc.xml" && \
  install -Dm644 "$S/labwc/menu.xml" "$STAGE$PREFIX/share/oss-greeter/labwc/menu.xml" && \
  install -Dm644 "$S/pam.d/oss-greeter" "$STAGE$PREFIX/etc/pam.d/oss-greeter"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- oss-greeter: the log-in screen of the QNX desktop -- the place of Ubuntu's GDM or Raspberry Pi OS' greeter. Written for this tree (files/), C and GTK 3 on gtk-layer-shell. qnx-desktop-start (qnx-desktop-core) shows it when qnx-desktop.conf has DESKTOP_GREETER=yes; by default the session still logs in by itself. Each round: libexec/oss-greeter/oss-greeter-session starts, as root, a labwc of its own on QNX Screen that runs only the greeter (no menu, no key or mouse binding: share/oss-greeter/labwc) and ends with it. The greeter lists root and the ordinary users of /etc/passwd, checks the password through PAM and writes its answer -- "login <user>", "reboot" or "poweroff" -- to /data/oss/var/run/oss-greeter/result (a directory only root can read); qnx-desktop-start then starts that user's session and shows the greeter again after Log Out. PAM: OpenPAM's policy etc/pam.d/oss-greeter (found in /usr/local/etc/pam.d) names QNX's own module by its path, /system/lib/dll/pam/pam_qnx.so, for the four facilities -- as swaylock's. pam_qnx.so links OpenPAM's libpam.so.2 from /usr/local/lib: the session script sets LD_LIBRARY_PATH (oss-env.sh).
- labwc runs it; the background is an SVG (librsvg's pixbuf loader).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH oss-greeter`.

### 5. What it installs

5 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/oss-greeter` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa oss-greeter
ls dists/8.0/*/ | grep '^oss-greeter-'
```

This writes `oss-greeter-<ver>-r0.tar.gz` (run time) and `oss-greeter-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add oss-greeter
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/oss-greeter-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/oss-greeter-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_oss_greeter`](../../tests/src/) links oss-greeter the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-oss-greeter`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-oss-greeter
/usr/local/bin/qnx-port-tests/test_oss_greeter
```

```
== oss-greeter ==
  ok   ...
PASS oss-greeter
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_oss_greeter` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_oss_greeter`](../../tests/src/) -- a program linked against oss-greeter the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> oss-greeter`.

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add oss-greeter=<version>`), sources in `ported/oss-greeter/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates oss-greeter`; port one with `scripts/port-update oss-greeter`.

<details><summary>recipe.sh</summary>

```bash
# oss-greeter: the log-in screen of the QNX desktop -- the place of Ubuntu's
# GDM or Raspberry Pi OS' greeter. Written for this tree (files/), C and
# GTK 3 on gtk-layer-shell.
#
# qnx-desktop-start (qnx-desktop-core) shows it when qnx-desktop.conf has
# DESKTOP_GREETER=yes; by default the session still logs in by itself. Each
# round: libexec/oss-greeter/oss-greeter-session starts, as root, a labwc of
# its own on QNX Screen that runs only the greeter (no menu, no key or mouse
# binding: share/oss-greeter/labwc) and ends with it. The greeter lists root
# and the ordinary users of /etc/passwd, checks the password through PAM and
# writes its answer -- "login <user>", "reboot" or "poweroff" -- to
# /data/oss/var/run/oss-greeter/result (a directory only root can read);
# qnx-desktop-start then starts that user's session and shows the greeter
# again after Log Out.
# PAM: OpenPAM's policy etc/pam.d/oss-greeter (found in /usr/local/etc/pam.d)
# names QNX's own module by its path, /system/lib/dll/pam/pam_qnx.so, for the
# four facilities -- as swaylock's. pam_qnx.so links OpenPAM's libpam.so.2
# from /usr/local/lib: the session script sets LD_LIBRARY_PATH (oss-env.sh).
VERSION=1.0
DEPS="gtk3 gtk-layer-shell openpam"
# labwc runs it; the background is an SVG (librsvg's pixbuf loader).
RUNTIME_DEPS="labwc openpam librsvg xfdesktop"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-greeter/files/." "$SRCROOT/"; }
do_build() {
  mkdir -p "$B" && cd "$B" && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -DPREFIX="\"$PREFIX\"" -o oss-greeter "$S/oss-greeter.c" \
      $(pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0) -lpam
}
do_install() {
  install -Dm755 "$B/oss-greeter" "$STAGE$PREFIX/libexec/oss-greeter/oss-greeter" && \
  install -Dm755 "$S/oss-greeter-session" "$STAGE$PREFIX/libexec/oss-greeter/oss-greeter-session" && \
  install -Dm644 "$S/labwc/rc.xml" "$STAGE$PREFIX/share/oss-greeter/labwc/rc.xml" && \
  install -Dm644 "$S/labwc/menu.xml" "$STAGE$PREFIX/share/oss-greeter/labwc/menu.xml" && \
  install -Dm644 "$S/pam.d/oss-greeter" "$STAGE$PREFIX/etc/pam.d/oss-greeter"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
