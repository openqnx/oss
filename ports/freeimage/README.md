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

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install freeimage
oss-get install freeimage-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH freeimage
ls repo/8.0/*/ | grep '^freeimage-'
```

This writes `freeimage-<ver>-r0.tar.gz` (run time) and `freeimage-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install freeimage
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/freeimage-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/freeimage-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
