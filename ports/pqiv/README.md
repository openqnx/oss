# pqiv 2.13.2

> Minimal command line image viewer based on GTK 3

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/phillipberndt/pqiv/archive/refs/tags/2.13.2.tar.gz |
| SHA-256 | `154cbbe0a62be12cee23b0a46a86b2305d8128fd19924308ad5e9d22fa5ad4f7` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`gdk-pixbuf`](../gdk-pixbuf/) [`cairo`](../cairo/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libx11`](../libx11/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pqiv`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `pqiv` (programs, shared libraries, run-time data) and `pqiv-dev` (headers, pkg-config/CMake files, static libraries; needs `pqiv`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`pqiv`](https://openqnx.github.io/oss/package/x86_64/pqiv.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pqiv.html)) · [`pqiv-dev`](https://openqnx.github.io/oss/package/x86_64/pqiv-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pqiv-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add pqiv` (dependencies come along); one version: `apk add pqiv=<version>` | `apk del pqiv` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install pqiv` (dependencies first) | `oss-get remove pqiv` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`pqiv-dev` the same way, by its own name (`apk add pqiv-dev`, `oss-get install pqiv-dev`, by hand with its own file and list `pqiv-dev.list`); remove it before `pqiv`, which it needs.

```sh
# on the QNX target, as root
apk add pqiv pqiv-dev
apk add qnx-test-pqiv && /usr/local/bin/qnx-port-tests/test_pqiv    # its on-target test, from tests-8.0-<arch>-m-z
apk del pqiv pqiv-dev qnx-test-pqiv
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/pqiv.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/pqiv.list
rm var/lib/manual/pqiv.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pqiv` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib gtk3 gdk-pixbuf cairo
```

<details><summary>Full build chain (58)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://github.com/phillipberndt/pqiv/archive/refs/tags/2.13.2.tar.gz
mkdir -p dist src/pqiv
curl -fL -o dist/$(basename $URL) $URL
echo "154cbbe0a62be12cee23b0a46a86b2305d8128fd19924308ad5e9d22fa5ad4f7  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/pqiv --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/pqiv < ports/pqiv/patches/0001-aports-remove-avclose.patch
```

**[`0001-aports-remove-avclose.patch`](patches/0001-aports-remove-avclose.patch)** — QNX change from QNX's aports (extra/pqiv/remove-avclose.patch).

> Taken from qnx-ports/aports extra/pqiv -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `backends/libav.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/pqiv; B=build/$QNX_ARCH/pqiv; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" PKG_CONFIG=pkg-config \
  ./configure --prefix="$PREFIX" --backends-build=static --backends=gdkpixbuf \
     ; }
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pqiv, a minimalist image viewer (GTK3). Version as in the QNX Everywhere 8.0.4r1 image.
- pqiv's configure is hand-written; it takes the compiler and the backends.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pqiv`.

### 5. What it installs

3 files under `/usr/local`, 1 programs.

```
/usr/local/bin/pqiv
```

The complete list is `stage/<arch>/.manifest/pqiv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pqiv
ls dists/8.0/*/ | grep '^pqiv-'
```

This writes `pqiv-<ver>-r0.tar.gz` (run time) and `pqiv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pqiv
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pqiv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pqiv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pqiv`](../../tests/src/) links pqiv the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pqiv`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pqiv
/usr/local/bin/qnx-port-tests/test_pqiv
```

```
== pqiv ==
  ok   ...
PASS pqiv
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pqiv` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pqiv`](../../tests/src/) -- a program linked against pqiv the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pqiv 2.13.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/pqiv-2.13.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pqiv 2.13.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/pqiv-2.13.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pqiv 2.13.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/pqiv-2.13.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pqiv 2.13.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/pqiv-2.13.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.13.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pqiv=<version>`), sources in `ported/pqiv/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pqiv`; port one with `scripts/port-update pqiv`.

<details><summary>recipe.sh</summary>

```bash
# pqiv, a minimalist image viewer (GTK3).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.13.2
TARBALL=https://github.com/phillipberndt/pqiv/archive/refs/tags/2.13.2.tar.gz
SHA256=154cbbe0a62be12cee23b0a46a86b2305d8128fd19924308ad5e9d22fa5ad4f7
DEPS="glib gtk3 gdk-pixbuf cairo"
BUILDSYS=custom
# pqiv's configure is hand-written; it takes the compiler and the backends.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" PKG_CONFIG=pkg-config \
  ./configure --prefix="$PREFIX" --backends-build=static --backends=gdkpixbuf \
     ; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
