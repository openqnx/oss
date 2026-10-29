# libyuv stable

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://chromium.googlesource.com/libyuv/libyuv (tag `stable`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) |
| Run-time dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libyuv`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libyuv
apk add libyuv-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libyuv && /usr/local/bin/qnx-port-tests/test_libyuv    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libyuv`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libyuv` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libjpeg-turbo
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b stable --recurse-submodules --shallow-submodules \
    https://chromium.googlesource.com/libyuv/libyuv src/libyuv
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libyuv < ports/libyuv/patches/0001-qnx-neon-asm-and-jpeg-link.patch
```

**[`0001-qnx-neon-asm-and-jpeg-link.patch`](patches/0001-qnx-neon-asm-and-jpeg-link.patch)** — libyuv: link libjpeg into the libraries; fix an aarch64 asm operand.

> With HAVE_JPEG the MJPEG decoder is compiled into libyuv itself, but only the
> yuvconvert tool linked libjpeg, leaving libyuv.so with unresolved jpeg_*
> symbols. In row_neon64.cc, ld1 takes its shuffle table through a register
> address operand: the "m" constraint gave GCC an addressing form ld1 does not
> accept.

Touches: `CMakeLists.txt`, `source/row_neon64.cc`, `source/scale_neon64.cc` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libyuv -B build/$QNX_ARCH/libyuv -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libyuv -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libyuv
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libyuv`.

### 5. What it installs

25 files under `/usr/local`: 1 shared-library entries, 1 programs, 22 headers.

```
/usr/local/lib/libyuv.so
/usr/local/bin/yuvconvert
```

The complete list is `stage/<arch>/.manifest/libyuv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libyuv
ls dists/8.0/*/ | grep '^libyuv-'
```

This writes `libyuv-<ver>-r0.tar.gz` (run time) and `libyuv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libyuv
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libyuv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libyuv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libyuv`](../../tests/src/) links libyuv the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libyuv`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libyuv
/usr/local/bin/qnx-port-tests/test_libyuv
```

```
== libyuv ==
  ok   ...
PASS libyuv
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libyuv` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libyuv`](../../tests/src/) -- a program linked against libyuv the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - ARGBToI420
   - I420ToARGB
   - ARGB -> I420 -> ARGB stays close
   - mean luma is preserved by scaling
   - row was mirrored
   - RotatePlane 90 degrees
   - rotation moved the bottom-left pixel to the top-left

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libyuv 0_git-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **stable**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libyuv=<version>`), sources in `ported/libyuv/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libyuv`; port one with `scripts/port-update libyuv`.

<details><summary>recipe.sh</summary>

```bash
VERSION=stable; GIT=https://chromium.googlesource.com/libyuv/libyuv; REF=stable
DEPS="libjpeg-turbo"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
