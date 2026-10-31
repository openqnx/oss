# pango 1.54.0

> Library for layout and rendering of internationalized text

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/pango.git (tag `1.54.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`harfbuzz`](../harfbuzz/) [`fribidi`](../fribidi/) [`cairo`](../cairo/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`libxft`](../libxft/) |
| Run-time dependencies | [`cairo`](../cairo/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`glib`](../glib/) [`harfbuzz`](../harfbuzz/) [`libx11`](../libx11/) [`libxft`](../libxft/) [`libxrender`](../libxrender/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pango`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `pango` (programs, shared libraries, run-time data) and `pango-dev` (headers, pkg-config/CMake files, static libraries; needs `pango`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`pango`](https://openqnx.github.io/oss/package/x86_64/pango.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pango.html)) · [`pango-dev`](https://openqnx.github.io/oss/package/x86_64/pango-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pango-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add pango` (dependencies come along); one version: `apk add pango=<version>` | `apk del pango` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install pango` (dependencies first) | `oss-get remove pango` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`pango-dev` the same way, by its own name (`apk add pango-dev`, `oss-get install pango-dev`, by hand with its own file and list `pango-dev.list`); remove it before `pango`, which it needs.

```sh
# on the QNX target, as root
apk add pango pango-dev
apk add qnx-test-pango && /usr/local/bin/qnx-port-tests/test_pango    # its on-target test
apk del pango pango-dev qnx-test-pango
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/pango.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/pango.list
rm var/lib/manual/pango.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pango` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (32 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib harfbuzz fribidi cairo fontconfig freetype libxft
```

<details><summary>Full build chain (32)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.54.0 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/pango.git src/pango
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/pango src/pango \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dgtk_doc=false \
      -Dintrospection=disabled \
      -Dbuild-testsuite=false \
      -Dbuild-examples=false \
      -Dcairo=enabled \
      -Dfontconfig=enabled \
      -Dfreetype=enabled \
      -Dlibthai=disabled \
      -Dsysprof=disabled
ninja -C build/$QNX_ARCH/pango
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/pango install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): libxft

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pango`.

### 5. What it installs

63 files under `/usr/local`: 12 shared-library entries, 3 programs, 38 headers.

```
/usr/local/lib/libpango-1.0.so
/usr/local/lib/libpango-1.0.so.0
/usr/local/lib/libpango-1.0.so.0.5400.0
/usr/local/lib/libpangocairo-1.0.so
/usr/local/lib/libpangocairo-1.0.so.0
/usr/local/lib/libpangocairo-1.0.so.0.5400.0
/usr/local/lib/libpangoft2-1.0.so
/usr/local/lib/libpangoft2-1.0.so.0
/usr/local/lib/libpangoft2-1.0.so.0.5400.0
/usr/local/lib/libpangoxft-1.0.so
/usr/local/lib/libpangoxft-1.0.so.0
/usr/local/lib/libpangoxft-1.0.so.0.5400.0
/usr/local/bin/pango-list
/usr/local/bin/pango-segmentation
/usr/local/bin/pango-view
```

The complete list is `stage/<arch>/.manifest/pango` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pango
ls dists/8.0/*/ | grep '^pango-'
```

This writes `pango-<ver>-r0.tar.gz` (run time) and `pango-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pango
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pango-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pango-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pango`](../../tests/src/) links pango the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pango`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pango
/usr/local/bin/qnx-port-tests/test_pango
```

```
== pango ==
  ok   ...
PASS pango
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pango` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pango`](../../tests/src/) -- a program linked against pango the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - pangocairo font map
   - pango_font_map_load_font(DejaVu Sans)
   - fontconfig found DejaVu Sans
   - font families listed
   - width = 10 x 1767/2048 x 20 px = 172.56 px
   - one line
   - no unknown glyphs
   - ten glyphs shaped
   - \"Hello QNX\" ~101 px wide (kerning aside)
   - xy_to_index(0,0) is the first character
   - word wrap at 100 px gives >= 3 lines
   - no line wider than 100 px
   - text was drawn
   - every drawn pixel is inside the ink rectangle
   - pango-segmentation word boundaries |Hello| |QNX|
   - pango-segmentation sentence boundaries
   - scratch directory
   - pango-view wrote a PNG
   - PNG is one line of 20 pt text

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pango 1.54.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/pango-1.54.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pango 1.54.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/pango-1.54.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pango 1.54.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/pango-1.54.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pango 1.54.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/pango-1.54.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.54.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pango=<version>`), sources in `ported/pango/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pango`; port one with `scripts/port-update pango`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.54.0; GIT=https://gitlab.gnome.org/GNOME/pango.git; REF=1.54.0
# Also linked at run time (found by scripts/gen-dependency-index): libxft
DEPS="glib harfbuzz fribidi cairo fontconfig freetype libxft"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dgtk_doc=false -Dintrospection=disabled -Dbuild-testsuite=false
                   -Dbuild-examples=false -Dcairo=enabled -Dfontconfig=enabled
                   -Dfreetype=enabled -Dlibthai=disabled -Dsysprof=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
