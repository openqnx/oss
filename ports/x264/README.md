# x264 stable

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://code.videolan.org/videolan/x264.git (tag `stable`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_x264`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install x264
oss-get install x264-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild x264` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b stable --recurse-submodules --shallow-submodules \
    https://code.videolan.org/videolan/x264.git src/x264
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/x264 < ports/x264/patches/0001-qnx8-no-msvc-extras.patch
```

**[`0001-qnx8-no-msvc-extras.patch`](patches/0001-qnx8-no-msvc-extras.patch)** — x264: do not put extras/ on the include path for QNX 8.0.

> extras/ holds MSVC-only shims (its inttypes.h is an #error for other
> compilers). Older QNX needed the getopt kept there; QNX 8.0 has a proper
> <inttypes.h>, which the shim shadowed. getopt is used only by the CLI, which
> is not built.

Touches: `configure` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/x264; B=build/$QNX_ARCH/x264; mkdir -p $B
```

```bash
do_configure() {
  # x264's aarch64 assembly is plain GNU as, so it builds here. Only the x86
  # paths need nasm, which this host lacks. --disable-asm is also a poorly
  # maintained path in x264 -- it drops struct members the C code still uses
  # ("x264_pixel_function_t has no member named ads"), so never take it on
  # aarch64.
  local asmopt=()
  [ "$QNX_ARCH" = x86_64 ] && { command -v nasm >/dev/null || asmopt=( --disable-asm ); }
  cd "$B"
  "$S/configure" --prefix="$PREFIX" \
    --host="$QNX_TRIPLE" \
    --cross-prefix="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
    --sysroot="$QNX_TARGET" \
    --enable-shared --disable-cli --disable-opencl \
    --extra-cflags="-D_QNX_SOURCE" \
    --extra-ldflags="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
    "${asmopt[@]}"
}
```

```bash
do_build()   { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- x264 has a hand-written configure that takes --host/--cross-prefix rather than the autoconf spelling, and it probes by compiling, so it needs the sysroot.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH x264`.

### 5. What it installs

5 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libx264.so
/usr/local/lib/libx264.so.165
```

The complete list is `stage/<arch>/.manifest/x264` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH x264
ls repo/8.0/*/ | grep '^x264-'
```

This writes `x264-<ver>-r0.tar.gz` (run time) and `x264-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install x264
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/x264-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/x264-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_x264`](../../tests/src/) links x264 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_x264 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_x264'
```

```
== x264 ==
  ok   ...
PASS x264
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=stable; GIT=https://code.videolan.org/videolan/x264.git; REF=stable
BUILDSYS=custom
# x264 has a hand-written configure that takes --host/--cross-prefix rather than
# the autoconf spelling, and it probes by compiling, so it needs the sysroot.
do_configure() {
  # x264's aarch64 assembly is plain GNU as, so it builds here. Only the x86
  # paths need nasm, which this host lacks. --disable-asm is also a poorly
  # maintained path in x264 -- it drops struct members the C code still uses
  # ("x264_pixel_function_t has no member named ads"), so never take it on
  # aarch64.
  local asmopt=()
  [ "$QNX_ARCH" = x86_64 ] && { command -v nasm >/dev/null || asmopt=( --disable-asm ); }
  cd "$B"
  "$S/configure" --prefix="$PREFIX" \
    --host="$QNX_TRIPLE" \
    --cross-prefix="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
    --sysroot="$QNX_TARGET" \
    --enable-shared --disable-cli --disable-opencl \
    --extra-cflags="-D_QNX_SOURCE" \
    --extra-ldflags="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -shared-libgcc" \
    "${asmopt[@]}"
}
do_build()   { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
