# freeimage 3.18.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add freeimage
apk add freeimage-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install freeimage`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
  make -f Makefile.gnu -j"$(nproc)" LIBRARIES="-lm"; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include" && install -m644 "$B/Source/FreeImage.h" "$STAGE$PREFIX/include/" && \
  install -m755 "$B"/libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/" && ln -sfn libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/libfreeimage.so.3" && \
  ln -sfn libfreeimage.so.3 "$STAGE$PREFIX/lib/libfreeimage.so"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Makefile.gnu appends its include paths with "CFLAGS += $(INCLUDE)", which a command-line CFLAGS would override; pass the flags through the environment. PNG_ARM_NEON_OPT=0: the bundled libpng turns its NEON row filters on for aarch64, but FreeImage's Makefile.gnu does not compile them (arm/*.c), so libfreeimage.so had png_init_filter_functions_neon undefined.

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

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | freeimage 3.18.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/freeimage-3.18.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | freeimage 3.18.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/freeimage-3.18.0-r0.log) |

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
do_build() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
  CFLAGS="-O2 -fPIC -D__ANSI__ -DDISABLE_PERF_MEASUREMENT -fvisibility=hidden -std=gnu99 -DPNG_ARM_NEON_OPT=0" \
  CXXFLAGS="-O2 -fPIC -std=gnu++14 -D__ANSI__ -DDISABLE_PERF_MEASUREMENT -fexceptions -fvisibility=hidden -Wno-ctor-dtor-privacy" \
  make -f Makefile.gnu -j"$(nproc)" LIBRARIES="-lm"; }
do_install() { install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include" && install -m644 "$B/Source/FreeImage.h" "$STAGE$PREFIX/include/" && \
  install -m755 "$B"/libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/" && ln -sfn libfreeimage-3.18.0.so "$STAGE$PREFIX/lib/libfreeimage.so.3" && \
  ln -sfn libfreeimage.so.3 "$STAGE$PREFIX/lib/libfreeimage.so"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
