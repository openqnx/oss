# freeimage 3.18.0

> Library for loading and saving popular image formats (PNG, JPEG, TIFF...)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_freeimage`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `freeimage` (programs, shared libraries, run-time data) and `freeimage-dev` (headers, pkg-config/CMake files, static libraries; needs `freeimage`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`freeimage`](https://openqnx.github.io/oss/package/x86_64/freeimage.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/freeimage.html)) · [`freeimage-dev`](https://openqnx.github.io/oss/package/x86_64/freeimage-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/freeimage-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add freeimage` (dependencies come along); one version: `apk add freeimage=<version>` | `apk del freeimage` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install freeimage` (dependencies first) | `oss-get remove freeimage` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`freeimage-dev` the same way, by its own name (`apk add freeimage-dev`, `oss-get install freeimage-dev`, by hand with its own file and list `freeimage-dev.list`); remove it before `freeimage`, which it needs.

```sh
# on the QNX target, as root
apk add freeimage freeimage-dev
apk add qnx-test-freeimage && /usr/local/bin/qnx-port-tests/test_freeimage    # its on-target test, from tests-8.0-<arch>
apk del freeimage freeimage-dev qnx-test-freeimage
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/freeimage.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/freeimage.list
rm var/lib/manual/freeimage.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild freeimage` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip
mkdir -p dist src/freeimage
curl -fL -o dist/$(basename $URL) $URL
tar xf dist/$(basename $URL) -C src/freeimage --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/freeimage; B=build/$QNX_ARCH/freeimage; mkdir -p $B
```

```bash
do_build() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
  CFLAGS="-O2 -fPIC -D__ANSI__ -DDISABLE_PERF_MEASUREMENT -fvisibility=hidden -std=gnu99 -DPNG_ARM_NEON_OPT=0" \
  CXXFLAGS="-O2 -fPIC -std=gnu++14 -D__ANSI__ -DDISABLE_PERF_MEASUREMENT -fexceptions -fvisibility=hidden -Wno-ctor-dtor-privacy" \
  make -f Makefile.gnu -j"$(nproc)" LIBRARIES="-lm -lc++"; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include" && install -m644 "$B/Source/FreeImage.h" "$STAGE$PREFIX/include/" && \
  install -m755 "$B"/libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/" && ln -sfn libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/libfreeimage.so.3" && \
  ln -sfn libfreeimage.so.3 "$STAGE$PREFIX/lib/libfreeimage.so"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Makefile.gnu appends its include paths with "CFLAGS += $(INCLUDE)", which a command-line CFLAGS would override; pass the flags through the environment. PNG_ARM_NEON_OPT=0: the bundled libpng turns its NEON row filters on for aarch64, but FreeImage's Makefile.gnu does not compile them (arm/*.c), so libfreeimage.so had png_init_filter_functions_neon undefined. LIBRARIES: Makefile.gnu links the shared library with $(CC) and upstream's "-lstdc++"; QNX's C++ runtime is libc++, and without naming it the library recorded no NEEDED for it, so a C program linking libfreeimage.so failed on undefined C++ runtime symbols.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH freeimage`.

### 5. What it installs

4 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libfreeimage-3.18.0.so
/usr/local/lib/libfreeimage.so
/usr/local/lib/libfreeimage.so.3
```

The complete list is `stage/<arch>/.manifest/freeimage` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa freeimage
ls dists/8.0/*/ | grep '^freeimage-'
```

This writes `freeimage-<ver>-r0.tar.gz` (run time) and `freeimage-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add freeimage
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/freeimage-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/freeimage-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_freeimage`](../../tests/src/) links freeimage the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-freeimage`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-freeimage
/usr/local/bin/qnx-port-tests/test_freeimage
```

```
== freeimage ==
  ok   ...
PASS freeimage
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_freeimage` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_freeimage`](../../tests/src/) -- a program linked against freeimage the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - allocate a 16x8 24-bit bitmap
   - pixels set and read back
   - FlipHorizontal moves column 0 to column 15
   - greyscale of pure red uses luma weights (~54 Rec.709 / ~76 Rec.601)
   - rescale a uniform image to 32x16 keeps its colour

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | freeimage 3.18.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/freeimage-3.18.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | freeimage 3.18.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/freeimage-3.18.0-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | freeimage 3.18.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/freeimage-3.18.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | freeimage 3.18.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/freeimage-3.18.0-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.18.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add freeimage=<version>`), sources in `ported/freeimage/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates freeimage`; port one with `scripts/port-update freeimage`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.18.0
TARBALL=https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" "$WS/dist" && local z="$WS/dist/FreeImage3180.zip"; [ -f "$z" ] || curl -fsSL -o "$z" "$TARBALL"; \
  cd "$SRCROOT" && unzip -q "$z" && shopt -s dotglob && mv FreeImage/* . && rmdir FreeImage; }
# Makefile.gnu appends its include paths with "CFLAGS += $(INCLUDE)", which a
# command-line CFLAGS would override; pass the flags through the environment.
# PNG_ARM_NEON_OPT=0: the bundled libpng turns its NEON row filters on for
# aarch64, but FreeImage's Makefile.gnu does not compile them (arm/*.c), so
# libfreeimage.so had png_init_filter_functions_neon undefined.
# LIBRARIES: Makefile.gnu links the shared library with $(CC) and upstream's
# "-lstdc++"; QNX's C++ runtime is libc++, and without naming it the library
# recorded no NEEDED for it, so a C program linking libfreeimage.so failed on
# undefined C++ runtime symbols.
do_build() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
  CFLAGS="-O2 -fPIC -D__ANSI__ -DDISABLE_PERF_MEASUREMENT -fvisibility=hidden -std=gnu99 -DPNG_ARM_NEON_OPT=0" \
  CXXFLAGS="-O2 -fPIC -std=gnu++14 -D__ANSI__ -DDISABLE_PERF_MEASUREMENT -fexceptions -fvisibility=hidden -Wno-ctor-dtor-privacy" \
  make -f Makefile.gnu -j"$(nproc)" LIBRARIES="-lm -lc++"; }
do_install() { install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include" && install -m644 "$B/Source/FreeImage.h" "$STAGE$PREFIX/include/" && \
  install -m755 "$B"/libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/" && ln -sfn libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/libfreeimage.so.3" && \
  ln -sfn libfreeimage.so.3 "$STAGE$PREFIX/lib/libfreeimage.so"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
