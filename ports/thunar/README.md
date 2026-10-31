# thunar 4.20.0

> Thunar, the Xfce file manager, and its extension library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/thunar/4.20/thunar-4.20.0.tar.bz2 |
| SHA-256 | `27731a76f3aecf3752b1ca35afad89e264c52244f70083d933507dd4a17548b0` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`exo`](../exo/) [`pango`](../pango/) [`gdk-pixbuf`](../gdk-pixbuf/) [`libnotify`](../libnotify/) [`libexif`](../libexif/) [`pcre2`](../pcre2/) [`xfce4-panel`](../xfce4-panel/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`exo`](../exo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libexif`](../libexif/) [`libnotify`](../libnotify/) [`libx11`](../libx11/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_thunar`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `thunar` (programs, shared libraries, run-time data) and `thunar-dev` (headers, pkg-config/CMake files, static libraries; needs `thunar`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`thunar`](https://openqnx.github.io/oss/package/x86_64/thunar.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/thunar.html)) · [`thunar-dev`](https://openqnx.github.io/oss/package/x86_64/thunar-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/thunar-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add thunar` (dependencies come along); one version: `apk add thunar=<version>` | `apk del thunar` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install thunar` (dependencies first) | `oss-get remove thunar` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`thunar-dev` the same way, by its own name (`apk add thunar-dev`, `oss-get install thunar-dev`, by hand with its own file and list `thunar-dev.list`); remove it before `thunar`, which it needs.

```sh
# on the QNX target, as root
apk add thunar thunar-dev
apk add qnx-test-thunar && /usr/local/bin/qnx-port-tests/test_thunar    # its on-target test
apk del thunar thunar-dev qnx-test-thunar
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/thunar.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/thunar.list
rm var/lib/manual/thunar.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild thunar` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel
```

<details><summary>Full build chain (62)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libexif`](../libexif/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libnotify`](../libnotify/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/thunar/4.20/thunar-4.20.0.tar.bz2
mkdir -p dist src/thunar
curl -fL -o dist/$(basename $URL) $URL
echo "27731a76f3aecf3752b1ca35afad89e264c52244f70083d933507dd4a17548b0  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/thunar --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/thunar && cd build/$QNX_ARCH/thunar
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/thunar/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-gtk-doc \
    --disable-introspection \
    --disable-gudev \
    --disable-polkit \
    --enable-notifications \
    --enable-exif \
    --enable-pcre2
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The Xfce file manager, and the thunarx extension library that its plugins (and tumbler's consumers) link against.  gudev is Linux's udev, which QNX does not have, so volume management is off; polkit likewise.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH thunar`.

### 5. What it installs

112 files under `/usr/local`: 2 shared-library entries, 3 programs, 14 headers.

```
/usr/local/lib/libthunarx-3.so
/usr/local/lib/libthunarx-3.so.0
/usr/local/bin/Thunar
/usr/local/bin/thunar
/usr/local/bin/thunar-settings
```

The complete list is `stage/<arch>/.manifest/thunar` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa thunar
ls dists/8.0/*/ | grep '^thunar-'
```

This writes `thunar-<ver>-r0.tar.gz` (run time) and `thunar-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add thunar
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/thunar-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/thunar-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_thunar`](../../tests/src/) links thunar the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-thunar`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-thunar
/usr/local/bin/qnx-port-tests/test_thunar
```

```
== thunar ==
  ok   ...
PASS thunar
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_thunar` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_thunar`](../../tests/src/) -- a program linked against thunar the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libthunarx 4.20.0
   - thunarx_check_version(4.20.0) accepts
   - thunarx_check_version(5.0.0): upstream stub, NULL
   - thunarx_menu_item_new
   - menu item properties read back
   - sensitive by default
   - set_sensitive(FALSE)
   - \"activate\" emitted on each thunarx_menu_item_activate
   - menu holds 3 items in order (prepend + 2 appends)
   - submenu attached to an item
   - scratch directory
   - thunar-sbr: ThunarSbrProvider (renamer provider) loaded
   - thunar-apr: ThunarAprProvider (property page provider) loaded
   - thunar-uca: ThunarUcaProvider (custom actions) loaded
   - thunar-wallpaper: TwpProvider loaded
   - thunar --version: thunar 4.20.0 (Xfce 4.20)
   - Thunar -V (the compatibility name)
   - thunar --help lists --bulk-rename, --daemon
   - default custom actions (uca.xml) installed

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | thunar 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/thunar-4.20.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | thunar 4.20.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/thunar-4.20.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | thunar 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/thunar-4.20.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | thunar 4.20.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/thunar-4.20.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add thunar=<version>`), sources in `ported/thunar/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates thunar`; port one with `scripts/port-update thunar`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.20.0
TARBALL=https://archive.xfce.org/src/xfce/thunar/4.20/thunar-4.20.0.tar.bz2
SHA256=27731a76f3aecf3752b1ca35afad89e264c52244f70083d933507dd4a17548b0
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel"
BUILDSYS=autotools
# The Xfce file manager, and the thunarx extension library that its plugins
# (and tumbler's consumers) link against.  gudev is Linux's udev, which QNX
# does not have, so volume management is off; polkit likewise.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-gudev --disable-polkit
                  --enable-notifications --enable-exif --enable-pcre2 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
