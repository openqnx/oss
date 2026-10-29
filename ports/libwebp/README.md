# libwebp 1.4.0

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://chromium.googlesource.com/webm/libwebp (tag `v1.4.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) [`giflib`](../giflib/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libwebp`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libwebp
apk add libwebp-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libwebp && /usr/local/bin/qnx-port-tests/test_libwebp    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libwebp`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libwebp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libpng libjpeg-turbo giflib
```

<details><summary>Full build chain (4)</summary>

[`giflib`](../giflib/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.4.0 --recurse-submodules --shallow-submodules \
    https://chromium.googlesource.com/webm/libwebp src/libwebp
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libwebp -B build/$QNX_ARCH/libwebp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DWEBP_BUILD_ANIM_UTILS=OFF \
      -DWEBP_BUILD_CWEBP=OFF \
      -DWEBP_BUILD_DWEBP=OFF \
      -DWEBP_BUILD_GIF2WEBP=OFF \
      -DWEBP_BUILD_IMG2WEBP=OFF \
      -DWEBP_BUILD_VWEBP=OFF \
      -DWEBP_BUILD_WEBPINFO=OFF \
      -DWEBP_BUILD_WEBPMUX=OFF \
      -DWEBP_BUILD_EXTRAS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libwebp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libwebp
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libwebp`.

### 5. What it installs

32 files under `/usr/local`: 15 shared-library entries, 8 headers.

```
/usr/local/lib/libsharpyuv.so
/usr/local/lib/libsharpyuv.so.0
/usr/local/lib/libsharpyuv.so.0.1.0
/usr/local/lib/libwebp.so
/usr/local/lib/libwebp.so.7
/usr/local/lib/libwebp.so.7.1.9
/usr/local/lib/libwebpdecoder.so
/usr/local/lib/libwebpdecoder.so.3
/usr/local/lib/libwebpdecoder.so.3.1.9
/usr/local/lib/libwebpdemux.so
/usr/local/lib/libwebpdemux.so.2
/usr/local/lib/libwebpdemux.so.2.0.15
/usr/local/lib/libwebpmux.so
/usr/local/lib/libwebpmux.so.3
/usr/local/lib/libwebpmux.so.3.1.0
```

The complete list is `stage/<arch>/.manifest/libwebp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libwebp
ls dists/8.0/*/ | grep '^libwebp-'
```

This writes `libwebp-<ver>-r0.tar.gz` (run time) and `libwebp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libwebp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libwebp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libwebp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libwebp`](../../tests/src/) links libwebp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libwebp`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libwebp
/usr/local/bin/qnx-port-tests/test_libwebp
```

```
== libwebp ==
  ok   ...
PASS libwebp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libwebp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libwebp`](../../tests/src/) -- a program linked against libwebp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - WebPEncodeLosslessRGBA
   - WebPGetInfo reports the right size
   - WebPDecodeRGBA
   - lossless round trip is byte-identical
   - WebPEncodeRGBA (lossy)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.4.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libwebp=<version>`), sources in `ported/libwebp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libwebp`; port one with `scripts/port-update libwebp`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.4.0; GIT=https://chromium.googlesource.com/webm/libwebp; REF=v1.4.0
DEPS="libpng libjpeg-turbo giflib"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DWEBP_BUILD_ANIM_UTILS=OFF
                   -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF -DWEBP_BUILD_GIF2WEBP=OFF
                   -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF -DWEBP_BUILD_WEBPINFO=OFF
                   -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_EXTRAS=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
