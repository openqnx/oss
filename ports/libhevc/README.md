# libhevc 1.8.5

> Android's HEVC (H.265) video decoder and encoder libraries

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/ittiam-systems/libhevc.git (tag `v1.8.5`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libhevc`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libhevc` (programs, shared libraries, run-time data) and `libhevc-dev` (headers, pkg-config/CMake files, static libraries; needs `libhevc`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libhevc`](https://openqnx.github.io/oss/package/x86_64/libhevc.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libhevc.html)) · [`libhevc-dev`](https://openqnx.github.io/oss/package/x86_64/libhevc-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libhevc-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libhevc` (dependencies come along); one version: `apk add libhevc=<version>` | `apk del libhevc` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libhevc` (dependencies first) | `oss-get remove libhevc` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libhevc-dev` the same way, by its own name (`apk add libhevc-dev`, `oss-get install libhevc-dev`, by hand with its own file and list `libhevc-dev.list`); remove it before `libhevc`, which it needs.

```sh
# on the QNX target, as root
apk add libhevc libhevc-dev
apk add qnx-test-libhevc && /usr/local/bin/qnx-port-tests/test_libhevc    # its on-target test, from tests-8.0-<arch>
apk del libhevc libhevc-dev qnx-test-libhevc
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libhevc.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libhevc.list
rm var/lib/manual/libhevc.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libhevc` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.8.5 --recurse-submodules --shallow-submodules \
    https://github.com/ittiam-systems/libhevc.git src/libhevc
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libhevc < ports/libhevc/patches/0001-cmake-enable-tests-option.patch
patch -p1 -d src/libhevc < ports/libhevc/patches/0002-encoder-qnx-thread-id.patch
```

**[`0001-cmake-enable-tests-option.patch`](patches/0001-cmake-enable-tests-option.patch)** — Make the googletest unit tests optional (ENABLE_TESTS).

> v1.8.5 fetches googletest (FetchContent, from github.com) at configure time
> and builds its unit tests unconditionally; a cross build for QNX can neither
> run them nor should it download anything while configuring. Put both behind
> an ENABLE_TESTS option (default ON, as upstream's development branch does
> since), which the recipe turns off.

Touches: `CMakeLists.txt` · Upstream status: Pending

**[`0002-encoder-qnx-thread-id.patch`](patches/0002-encoder-qnx-thread-id.patch)** — Encoder OS layer: no Linux headers, pthread_self() as thread id on QNX.

> The encoder's OS layer includes <linux/unistd.h> and <sys/syscall.h> and
> gets the current thread id with syscall(__NR_gettid) on everything but
> Darwin. QNX has neither header nor a gettid system call; its POSIX thread
> ids (pthread_self()) are small integers unique within the process, which is
> what the encoder uses the id for.

Touches: `encoder/osal_thread.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libhevc; B=build/$QNX_ARCH/libhevc; mkdir -p $B
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/libhevc" "$STAGE$PREFIX/bin" &&
  install -m 644 "$B/libhevcdec.a" "$B/libhevcenc.a" "$STAGE$PREFIX/lib/" &&
  install -m 644 "$S/common/ihevc_typedefs.h" "$S/common/iv.h" "$S/common/ivd.h" \
                 "$S/decoder/ihevcd_cxa.h" "$S/encoder/ihevce_api.h" "$S/encoder/ihevce_plugin.h" \
                 "$S/encoder/itt_video_api.h" "$STAGE$PREFIX/include/libhevc/" &&
  install -m 755 "$B/hevcdec" "$B/hevcenc" "$STAGE$PREFIX/bin/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libhevc: Android's HEVC (H.265) video decoder and encoder libraries (from Ittiam, the code behind Android's software HEVC codecs), with the hevcdec and hevcenc example programs.
- SYSTEM_NAME/SYSTEM_PROCESSOR pick the SIMD code; they default to the build machine's. aarch64: the NEON assembly and intrinsics; x86_64: the SSSE3/SSE4.2 intrinsics (the project builds everything with -msse4.2 -- any x86_64 board QNX 8 runs on has it). Tests and benchmarks would download googletest and google-benchmark at configure time: off. PIC so that the static libraries can be linked into shared objects.
- Upstream (like Android) builds static libraries and installs nothing: install the two libraries, their public API headers (in include/libhevc) and the two example programs.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libhevc`.

### 5. What it installs

11 files under `/usr/local`, 2 programs, 7 headers.

```
/usr/local/bin/hevcdec
/usr/local/bin/hevcenc
```

The complete list is `stage/<arch>/.manifest/libhevc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libhevc
ls dists/8.0/*/ | grep '^libhevc-'
```

This writes `libhevc-<ver>-r0.tar.gz` (run time) and `libhevc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libhevc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libhevc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libhevc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libhevc`](../../tests/src/) links libhevc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libhevc`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libhevc
/usr/local/bin/qnx-port-tests/test_libhevc
```

```
== libhevc ==
  ok   ...
PASS libhevc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libhevc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libhevc`](../../tests/src/) -- a program linked against libhevc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - decoder created (4:2:0 planar output)
   - IVD_CMD_CTL_GETVERSION names the HEVC decoder
   - one core
   - header decode mode
   - header decode: 32x32 picture
   - frame decode mode
   - flush
   - 3 pictures decoded
   - every sample equals the generating formula (lossless)
   - a stream cut inside the headers yields no pictures
   - scratch directory
   - hevcdec exits 0
   - hevcdec output: 4608 bytes, identical to the source frames

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libhevc 1.8.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libhevc-1.8.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libhevc 1.8.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libhevc-1.8.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.8.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libhevc=<version>`), sources in `ported/libhevc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libhevc`; port one with `scripts/port-update libhevc`.

<details><summary>recipe.sh</summary>

```bash
# libhevc: Android's HEVC (H.265) video decoder and encoder libraries (from
# Ittiam, the code behind Android's software HEVC codecs), with the hevcdec
# and hevcenc example programs.
VERSION=1.8.5; GIT=https://github.com/ittiam-systems/libhevc.git; REF=v1.8.5
BUILDSYS=cmake
# SYSTEM_NAME/SYSTEM_PROCESSOR pick the SIMD code; they default to the build
# machine's. aarch64: the NEON assembly and intrinsics; x86_64: the SSSE3/SSE4.2
# intrinsics (the project builds everything with -msse4.2 -- any x86_64 board
# QNX 8 runs on has it). Tests and benchmarks would download googletest and
# google-benchmark at configure time: off. PIC so that the static libraries
# can be linked into shared objects.
CMAKE_ARGS_EXTRA=( -DENABLE_TESTS=OFF -DENABLE_BENCHMARKS=OFF -DSYSTEM_NAME=QNX
                   -DSYSTEM_PROCESSOR=$( [ "$QNX_ARCH" = aarch64le ] && echo aarch64 || echo x86_64 )
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
# Upstream (like Android) builds static libraries and installs nothing:
# install the two libraries, their public API headers (in include/libhevc)
# and the two example programs.
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/libhevc" "$STAGE$PREFIX/bin" &&
  install -m 644 "$B/libhevcdec.a" "$B/libhevcenc.a" "$STAGE$PREFIX/lib/" &&
  install -m 644 "$S/common/ihevc_typedefs.h" "$S/common/iv.h" "$S/common/ivd.h" \
                 "$S/decoder/ihevcd_cxa.h" "$S/encoder/ihevce_api.h" "$S/encoder/ihevce_plugin.h" \
                 "$S/encoder/itt_video_api.h" "$STAGE$PREFIX/include/libhevc/" &&
  install -m 755 "$B/hevcdec" "$B/hevcenc" "$STAGE$PREFIX/bin/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
