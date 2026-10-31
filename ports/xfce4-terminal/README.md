# xfce4-terminal 1.1.5

> Xfce terminal emulator

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/apps/xfce4-terminal/1.1/xfce4-terminal-1.1.5.tar.xz |
| SHA-256 | `3c5b1d3a01a9a113852ac0f77d1c85bf3a356b43de33ec805b21ceca7d6f0a63` |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`vte`](../vte/) [`pcre2`](../pcre2/) [`gtk-layer-shell`](../gtk-layer-shell/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`glib`](../glib/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`vte`](../vte/) [`xfconf`](../xfconf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xfce4_terminal`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `xfce4-terminal`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`xfce4-terminal`](https://openqnx.github.io/oss/package/x86_64/xfce4-terminal.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xfce4-terminal.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add xfce4-terminal` (dependencies come along); one version: `apk add xfce4-terminal=<version>` | `apk del xfce4-terminal` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install xfce4-terminal` (dependencies first) | `oss-get remove xfce4-terminal` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-xfce4-terminal && /usr/local/bin/qnx-port-tests/test_xfce4_terminal    # its on-target test
apk del xfce4-terminal qnx-test-xfce4-terminal
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/xfce4-terminal.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/xfce4-terminal.list
rm var/lib/manual/xfce4-terminal.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfce4-terminal` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (69 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell
```

<details><summary>Full build chain (69)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`gtk4`](../gtk4/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtasn1`](../libtasn1/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`vte`](../vte/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/apps/xfce4-terminal/1.1/xfce4-terminal-1.1.5.tar.xz
mkdir -p dist src/xfce4-terminal
curl -fL -o dist/$(basename $URL) $URL
echo "3c5b1d3a01a9a113852ac0f77d1c85bf3a356b43de33ec805b21ceca7d6f0a63  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfce4-terminal --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xfce4-terminal < ports/xfce4-terminal/patches/0001-aports-0001-add-meson-option-to-build-doc.patch
patch -p1 -d src/xfce4-terminal < ports/xfce4-terminal/patches/0002-aports-0002-remove-opening-file-manager-option.patch
```

**[`0001-aports-0001-add-meson-option-to-build-doc.patch`](patches/0001-aports-0001-add-meson-option-to-build-doc.patch)** — QNX change from QNX's aports (extra/xfce4-terminal/0001-add-meson-option-to-build-doc.patch).

> Taken from qnx-ports/aports extra/xfce4-terminal -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> add meson option to build doc
>
> ---
>  meson.build       | 4 +++-
>  meson_options.txt | 7 +++++++
>  2 files changed, 10 insertions(+), 1 deletion(-)

Touches: `meson.build`, `meson_options.txt` · Upstream status: QNX-specific

**[`0002-aports-0002-remove-opening-file-manager-option.patch`](patches/0002-aports-0002-remove-opening-file-manager-option.patch)** — QNX change from QNX's aports (extra/xfce4-terminal/0002-remove-opening-file-manager-option.patch).

> Taken from qnx-ports/aports extra/xfce4-terminal -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> remove opening file manager option
>
> ---
>  terminal/terminal-window.c | 6 ++++++
>  1 file changed, 6 insertions(+)

Touches: `terminal/terminal-window.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xfce4-terminal src/xfce4-terminal \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dx11=disabled \
      -Dwayland=enabled \
      -Ddocs=false
ninja -C build/$QNX_ARCH/xfce4-terminal
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xfce4-terminal install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xfce4-terminal: terminal emulator (VTE). Version as in the QNX Everywhere 8.0.4r1 image; it builds against the Xfce 4.20 libraries in this tree.
- Also linked at run time (found by scripts/gen-dependency-index): gtk-layer-shell

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfce4-terminal`.

### 5. What it installs

86 files under `/usr/local`, 1 programs.

```
/usr/local/bin/xfce4-terminal
```

The complete list is `stage/<arch>/.manifest/xfce4-terminal` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfce4-terminal
ls dists/8.0/*/ | grep '^xfce4-terminal-'
```

This writes `xfce4-terminal-<ver>-r0.tar.gz` (run time) and `xfce4-terminal-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfce4-terminal
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfce4-terminal-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfce4-terminal-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xfce4_terminal`](../../tests/src/) links xfce4-terminal the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xfce4-terminal`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xfce4-terminal
/usr/local/bin/qnx-port-tests/test_xfce4_terminal
```

```
== xfce4-terminal ==
  ok   ...
PASS xfce4-terminal
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xfce4_terminal` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xfce4_terminal`](../../tests/src/) -- a program linked against xfce4-terminal the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xfce4-terminal 1.1.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xfce4-terminal-1.1.5-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xfce4-terminal 1.1.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/xfce4-terminal-1.1.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xfce4-terminal 1.1.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xfce4-terminal-1.1.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xfce4-terminal 1.1.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/xfce4-terminal-1.1.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.1.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfce4-terminal=<version>`), sources in `ported/xfce4-terminal/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfce4-terminal`; port one with `scripts/port-update xfce4-terminal`.

<details><summary>recipe.sh</summary>

```bash
# xfce4-terminal: terminal emulator (VTE). Version as in the QNX Everywhere
# 8.0.4r1 image; it builds against the Xfce 4.20 libraries in this tree.
VERSION=1.1.5
TARBALL=https://archive.xfce.org/src/apps/xfce4-terminal/1.1/xfce4-terminal-1.1.5.tar.xz
SHA256=3c5b1d3a01a9a113852ac0f77d1c85bf3a356b43de33ec805b21ceca7d6f0a63
# Also linked at run time (found by scripts/gen-dependency-index): gtk-layer-shell
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled -Ddocs=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
