# x264 stable

> H.264/MPEG-4 AVC video encoder library

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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add x264
apk add x264-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-x264 && /usr/local/bin/qnx-port-tests/test_x264    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install x264`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa x264
ls dists/8.0/*/ | grep '^x264-'
```

This writes `x264-<ver>-r0.tar.gz` (run time) and `x264-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add x264
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/x264-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/x264-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_x264`](../../tests/src/) links x264 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-x264`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-x264
/usr/local/bin/qnx-port-tests/test_x264
```

```
== x264 ==
  ok   ...
PASS x264
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_x264` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_x264`](../../tests/src/) -- a program linked against x264 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - x264_param_default_preset
   - apply baseline profile
   - x264_encoder_open
   - x264_picture_alloc
   - SPS/PPS headers emitted
   - all frames encoded
   - bitstream bytes produced

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | x264 0_git-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/x264-0_git-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | x264 0_git-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/x264-0_git-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **stable**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add x264=<version>`), sources in `ported/x264/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates x264`; port one with `scripts/port-update x264`.

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
