# libvpx 1.15.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://chromium.googlesource.com/webm/libvpx (tag `v1.15.0`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libvpx`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libvpx
oss-get install libvpx-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libvpx` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.15.0 --recurse-submodules --shallow-submodules \
    https://chromium.googlesource.com/webm/libvpx src/libvpx
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libvpx; B=build/$QNX_ARCH/libvpx; mkdir -p $B
```

```bash
do_configure() {
  local tgt extra=()
  case "$QNX_ARCH" in
    aarch64le) tgt=arm64-linux-gcc      # generic ELF/aarch64 profile; CROSS gives the real toolchain
               # libvpx's aarch64 runtime CPU detection reads Linux hwcaps
               # (getauxval); there is no QNX method, and the file says so:
               # "Rerun configure with --disable-runtime-cpu-detect". Without
               # detection every extension that is compiled in is called
               # unconditionally, so keep to what all ARMv8.0 parts have: the
               # dotprod/i8mm/SVE paths died with SIGILL on a Cortex-A57.
               extra+=( --disable-runtime-cpu-detect --disable-neon_dotprod --disable-neon_i8mm
                        --disable-sve --disable-sve2 ) ;;
    x86_64)    tgt=x86_64-linux-gcc ;;  # x86 detection is cpuid, which works anywhere
  esac
  cd "$B"
  CROSS="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
  CFLAGS="-D_QNX_SOURCE" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
  "$S/configure" --prefix="$PREFIX" --target="$tgt" \
    --enable-pic --enable-shared --disable-static \
    --disable-examples --disable-tools --disable-docs --disable-unit-tests \
    --enable-vp8 --enable-vp9 --disable-webm-io \
    "${extra[@]}"
}
```

```bash
do_build()   { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DIST_DIR="$STAGE$PREFIX" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libvpx has a hand-written configure driven by a --target triple of its own.
- libvpx installs into DIST_DIR, not DESTDIR: passing DESTDIR alone silently produces no files in the staging tree.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libvpx`.

### 5. What it installs

16 files under `/usr/local`: 4 shared-library entries, 11 headers.

```
/usr/local/lib/libvpx.so
/usr/local/lib/libvpx.so.9
/usr/local/lib/libvpx.so.9.1
/usr/local/lib/libvpx.so.9.1.0
```

The complete list is `stage/<arch>/.manifest/libvpx` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libvpx
ls repo/8.0/*/ | grep '^libvpx-'
```

This writes `libvpx-<ver>-r0.tar.gz` (run time) and `libvpx-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libvpx
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libvpx-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libvpx-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libvpx`](../../tests/src/) links libvpx the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libvpx root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libvpx'
```

```
== libvpx ==
  ok   ...
PASS libvpx
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.15.0; GIT=https://chromium.googlesource.com/webm/libvpx; REF=v1.15.0
BUILDSYS=custom
# libvpx has a hand-written configure driven by a --target triple of its own.
do_configure() {
  local tgt extra=()
  case "$QNX_ARCH" in
    aarch64le) tgt=arm64-linux-gcc      # generic ELF/aarch64 profile; CROSS gives the real toolchain
               # libvpx's aarch64 runtime CPU detection reads Linux hwcaps
               # (getauxval); there is no QNX method, and the file says so:
               # "Rerun configure with --disable-runtime-cpu-detect". Without
               # detection every extension that is compiled in is called
               # unconditionally, so keep to what all ARMv8.0 parts have: the
               # dotprod/i8mm/SVE paths died with SIGILL on a Cortex-A57.
               extra+=( --disable-runtime-cpu-detect --disable-neon_dotprod --disable-neon_i8mm
                        --disable-sve --disable-sve2 ) ;;
    x86_64)    tgt=x86_64-linux-gcc ;;  # x86 detection is cpuid, which works anywhere
  esac
  cd "$B"
  CROSS="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
  CFLAGS="-D_QNX_SOURCE" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
  "$S/configure" --prefix="$PREFIX" --target="$tgt" \
    --enable-pic --enable-shared --disable-static \
    --disable-examples --disable-tools --disable-docs --disable-unit-tests \
    --enable-vp8 --enable-vp9 --disable-webm-io \
    "${extra[@]}"
}
do_build()   { make -C "$B" -j"$(nproc)"; }
# libvpx installs into DIST_DIR, not DESTDIR: passing DESTDIR alone silently
# produces no files in the staging tree.
do_install() { make -C "$B" DIST_DIR="$STAGE$PREFIX" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
